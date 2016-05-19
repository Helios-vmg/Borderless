is_pure_filter = true

local use_old_traversal = false

function make_f(colors, radius, nosteps)
	return function(x)
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
	local t0 = os.clock()
	local w, h
	w, h = get_image_dimensions(img)
	local colors = {}
	local steps = 2
	for i = 1, steps do
		colors[i] = 255 / (steps - 1) * (i - 1)
	end
	local f = make_f(colors, 0, false)

	local pixels = {}
	traverse_image(
		img,
		function (r, g, b, a, x, y)
			local luma = r * 0.2126 + g * 0.7152 + b * 0.0722
			pixels[x + y * w] = luma
		end
	)
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
			local newpixel = f(oldpixel)
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
	traverse_image(
		img,
		function (r, g, b, a, x, y)
			local c = pixels[x + y * w]
			set_current_pixel(c, c, c, a)
		end
	)
	local t1 = os.clock()
	show_message_box("Elapsed time: " .. (t1 - t0) .. " s")
end
