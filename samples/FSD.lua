jit.opt.start(2)
local ffi = require("ffi")
ffi.cdef[[
struct position_info{
	unsigned char rgba[4];
	int x;
	int y;
};

void *new_traversal_iterator_c(int imgno);
void free_traversal_iterator_c(void *p);
bool traversal_iterator_next_c(void *p);
struct position_info traversal_iterator_get_c(void *p);
void traversal_iterator_set_array_c(void *p, unsigned char rgba[4]);
void traversal_iterator_set_c(void *p, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void traversal_iterator_reset_c(void *p);
]]
is_pure_filter = true

local use_old_traversal = false

function make_f(colors, radius)
	return function(x, nosteps)
		local pos = x / 255 * (#colors - 1) + 1
		local low = math.floor(pos)
		local high = math.ceil(pos)
		local which = math.random() < ((pos - low) - radius) / (1 - radius) and high or low
		if nosteps then
			which = math.floor(pos + 0.5)
		end
		return colors[which]
	end
end

function main(img)
	local C = ffi.C
	local t0 = os.clock()
	local w, h
	w, h = get_image_dimensions(img)
	local colors = {}
	local steps = 2
	for i = 1, steps do
		colors[i] = 255 / (steps - 1) * (i - 1)
	end
	local radius = 0
	local f = make_f(colors, radius)

	local pixels = {}
	if use_old_traversal then
		traverse_image(
			img,
			function (r, g, b, a, x, y)
				local luma = r * 0.2126 + g * 0.7152 + b * 0.0722
				pixels[x + y * w] = luma
			end
		)
	else
		--[[
		do
			local iterator = new_traversal_iterator(img)
			while traversal_iterator_next(iterator) do
				local r, g, b, a, x, y
				r, g, b, a, x, y = traversal_iterator_get(iterator)
				local luma = r * 0.2126 + g * 0.7152 + b * 0.0722
				pixels[x + y * w] = luma
			end
			free_traversal_iterator(iterator)
		end
		]]--
		--local pi = ffi.new("struct position_info")
		local iterator = C.new_traversal_iterator_c(img)
		while C.traversal_iterator_next_c(iterator) do
			local pi = C.traversal_iterator_get_c(iterator)
			local luma = pi.rgba[0] * 0.2126 + pi.rgba[1] * 0.7152 + pi.rgba[2] * 0.0722
			pixels[pi.x + pi.y * w] = luma
		end
		C.free_traversal_iterator_c(iterator)
	end
	local offsets = {
		{  1, 0 },
		{ -1, 1 },
		{  0, 1 },
		{  1, 1 }
	}
	local weights = {
		{    0,    0,    0,    0 },
		{    0,    1,    0,    0 },
		{ 7/16,    0,    0,    0 },
		{ 8/16,    0, 6/16, 2/16 },
		{    0,    0,    0,    0 },
		{    0, 7/16, 9/16,    0 },
		{ 7/16,    0,    0,    0 },
		{ 7/16, 3/16, 5/16, 1/16 }
	}
	for y = 0, h - 1 do
		local start = 0
		local finish = w - 1
		local invert = y % 2 ~= 0
		local step = invert and -1 or 1
		if invert then
			start, finish = finish, start
		end
		for x = start, finish, step do
			local oldpixel = pixels[x + y * w]
			if oldpixel > 255 then
				oldpixel = 255
			else
				if oldpixel < 0 then
					oldpixel = 0
				end
			end
			local newpixel = f(oldpixel, true)
			pixels[x + y * w] = newpixel
			local qe = oldpixel - newpixel
			local weight_chosen
			if not invert then
				weight_chosen = (x == 0 and 0 or 4) + (x == w - 1 and 0 or 2) + (y == h - 1 and 0 or 1) + 1
			else
				weight_chosen = (x == 0 and 0 or 2) + (x == w - 1 and 0 or 4) + (y == h - 1 and 0 or 1) + 1
			end
			for i = 1, 4 do
				local weight = weights[weight_chosen][i]
				if weight > 0 then
					local x2 = x + offsets[i][1] * (invert and -1 or 1)
					local y2 = y + offsets[i][2]
					local j = x2 + y2 * w
					if (not invert and y2 == y and x2 < x) or (invert and y2 == y and x2 > x) then
						print("ERROR ", x, y, x2, y2, invert)
					end
					pixels[j] = pixels[j] + qe * weight
				end
			end
		end
	end
	if use_old_traversal then
		traverse_image(
			img,
			function (r, g, b, a, x, y)
				local c = pixels[x + y * w]
				set_current_pixel(c, c, c, a)
			end
		)
	else
		--[[
		local iterator = new_traversal_iterator(img)
		while traversal_iterator_next(iterator) do
			local r, g, b, a, x, y
			r, g, b, a, x, y = traversal_iterator_get(iterator)
			local c = pixels[x + y * w]
			traversal_iterator_set(iterator, c, c, c, a)
		end
		free_traversal_iterator(iterator)
		]]--
		local iterator = C.new_traversal_iterator_c(img)
		while C.traversal_iterator_next_c(iterator) do
			local pi = C.traversal_iterator_get_c(iterator)
			local c = pixels[pi.x + pi.y * w]
			C.traversal_iterator_set_c(iterator, c, c, c, pi.rgba[3])
		end
		C.free_traversal_iterator_c(iterator)
	end
	local t1 = os.clock()
	show_message_box("Elapsed time: " .. (t1 - t0) .. " s")
end
