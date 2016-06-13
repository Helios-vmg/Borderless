local t0 = os.clock()
local img = get_displayed_image()
local w, h
w, h = get_image_dimensions(img)
local colors = {}
local steps = 2
for i = 1, steps do
	colors[i] = 255 / (steps - 1) * (i - 1)
end
local radius = 0

function f(x, nosteps)
	local pos = x / 255 * (#colors - 1) + 1
	local low = math.floor(pos)
	local high = math.ceil(pos)
	local which = math.random() < ((pos - low) - radius) / (1 - radius) and high or low
	if nosteps then
		which = math.floor(pos + 0.5)
	end
	return colors[which]
end

print("Loading image data.")
local pixels = {}
traverse_image(
	img,
	function (r, g, b, a, x, y)
		local luma = r * 0.2126 + g * 0.7152 + b * 0.0722
		--luma = y / (h - 1) * 255
		pixels[x + y * w] = luma
	end
)
print("Computing new data.")

function create_weights()
	return {
		{
			offset = { 1, 0 },
			weight = 7/16
		},
		{
			offset = { 1, 1 },
			weight = 2/16
		},
		{
			offset = { 0, 1 },
			weight = 7/16
		}
	}
end

do
	local x, y, state
	x = 0
	y = 0
	state = 0
	while true do
		x, y, state = zig_zag_order(x, y, w, h, state)
		if state < 0 then
			break
		end
		
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
		local weights = create_weights()
		local unusable = 0
		local sum = 0
		for i = 1, #weights do
			local x2 = x + weights[i].offset[1]
			local y2 = y + weights[i].offset[2]
			if x2 >= w or x2 < 0 or y2 >= h or y2 < 0 then
				unusable = unusable + weights[i].weight
				weights[i].weight = 0
			else
				sum = sum + weights[i].weight
			end
		end
		for i = 1, #weights do
			weights[i].weight = weights[i].weight + weights[i].weight / sum * unusable
		end
		for i = 1, #weights do
			local weight = weights[i].weight
			if weight > 0 then
				local x2 = x + weights[i].offset[1]
				local y2 = y + weights[i].offset[2]
				local j = x2 + y2 * w
				pixels[j] = pixels[j] + qe * weight
			end
		end
	end
end
print("Applying new data.")
traverse_image(
	img,
	function (r, g, b, a, x, y)
		local c = pixels[x + y * w]
		set_current_pixel(c, c, c, 255)
	end
)
print("Saving image data.")
display_in_current_window(img)
unload_image(img)
local t1 = os.clock()
print("Elapsed time: ", t1 - t0, " s")
