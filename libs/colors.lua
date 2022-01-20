local _M = {}

function _M.rgb_to_hsv(red, green, blue)
	local hue, saturation, value

	local min_value = math.min(red, green, blue)
	local max_value = math.max(red, green, blue)
	value = max_value

	local value_delta = max_value - min_value

	-- If the color is not black
	if max_value ~= 0 then
		saturation = value_delta / max_value

	-- If the color is purely black
	else
		saturation = 0
		hue = -1
		return hue, saturation, value
	end

	if red == max_value then
		hue = (green - blue) / value_delta
	elseif green == max_value then
		hue = 2 + (blue - red) / value_delta
	else
		hue = 4 + (red - green) / value_delta
	end

	hue = hue * 60
	if hue < 0 then
		hue = hue + 360
	end

	return hue, saturation, value
end

function _M.hsv_to_rgb(h, s, v)
	local r, g, b
	local i
	local f, p, q, t

	if s == 0 then
		r = v
		g = v
		b = v
		return r, g, b
	end

	h = h / 60
	i = math.floor(h)
	f = h - i
	p = v * (1 - s)
	q = v * (1 - s * f)
	t = v * (1 - s * (1 - f))
	if i == 0 then
		r = v
		g = t
		b = p
	elseif i == 1 then
		r = q
		g = v
		b = p
	elseif i == 2 then
		r = p
		g = v
		b = t
	elseif i == 3 then
		r = p
		g = q
		b = v
	elseif i == 4 then
		r = t
		g = p
		b = v
	elseif i == 5 then
		r = v
		g = p
		b = q
	end
	return r, g, b
end

function _M.rgb(r, g, b)
	return vmath.vector4(r / 255, g / 255, b / 255, 1)
end

return _M
