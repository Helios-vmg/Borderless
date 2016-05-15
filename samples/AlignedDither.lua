function gcd(a, b)
	while b ~= 0 do
		--debug_print(a .. ", " .. b)
		local t = b
		b = a % b
		a = t
	end
	--debug_print(a)
	return a
end

function reduce_fraction(a, b)
	local c = gcd(a, b)
	return a / c, b / c
end

local img = get_displayed_image()
w, h = get_image_dimensions(img)

function generate_bayer_matrix(power)
	local ret = {}
	if power == 0 then
		ret[0] = 0;
		return ret
	end
	local side = 2^power
	local last = generate_bayer_matrix(power - 1)
	local last_side = 2^(power-1)
	local offsets = {
		{0, 0},
		{1, 1},
		{0, 1},
		{1, 0},
	}
	for quadrant = 0, 3 do
		local offx = offsets[quadrant + 1][1] * last_side
		local offy = offsets[quadrant + 1][2] * last_side
		for y = 0, last_side - 1 do
			for x = 0, last_side - 1 do
				local dst = x + offx + (y + offy) * side
				local src = x + y * last_side
				ret[dst] = last[src] * 4 + quadrant;
			end
		end
	end
	return ret
end

local pattern = generate_bayer_matrix(4)

function dither_pixel(r, g, b, a, x, y)
	local luma = r * 0.2126 + g * 0.7152 + b * 0.0722
	luma = math.floor(luma + 0.5)
	local c = 0
	local i = x % 16 + y % 16 * 16
	if luma >= pattern[i] then
		c = 255
	end
	return c, c, c, a
end

traverse_image(
	img,
	function (r, g, b, a, x, y)
		set_current_pixel(dither_pixel(r, g, b, a, x, y))
	end
)

display_in_current_window(img)
unload_image(img)
