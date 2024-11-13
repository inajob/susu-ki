screenWidth = screenwidth()
screenHeight = screenheight()
fontHeight = 16

line = ""
result = ""
function subChar(s, start, e)
    local counter = 1
    local r = ""
    for p, c in utf8.codes(s) do
        if counter >= start and counter < e then
            r = r .. utf8.char(c)
        end
        counter = counter + 1
    end
    return r
end

function split(string, delim)
	local stringTable = {}
	local lastIndex = 1
	for i=1,string.len(string) do
		local currentlyContains = true
		for j=1,string.len(delim) do
			if (string.sub(string, i+j-1, i+j-1) ~= string.sub(delim, j, j)) then
				currentlyContains = false
			end
		end
		if (currentlyContains and string.len(string.sub(string, lastIndex, i-1)) > 0) then
			stringTable[#stringTable+1] = string.sub(string, lastIndex, i-1)
			lastIndex = i+string.len(delim)
		end
	end
	stringTable[#stringTable+1] = string.sub(string, lastIndex, #string)
	return stringTable
end

function draw(setPos)
    color(255,255,255)
    fillrect(0, 0, screenWidth, screenHeight)

    color(100,100,255)
    fillrect(0, 0, screenWidth, fontHeight)
    color(150,150,255)
    fillrect(2, 2, screenWidth - 4, fontHeight - 4)
    color(255,255,255)
    text("== Shell ==", 2, 2)

    color(0,0,0)
    local l = "> " .. line
    text(l, 0, fontHeight * 1)

    -- cursor
    local w = textwidth(l)
    fillrect(w, fontHeight, 2, fontHeight)

    text(result, 0, fontHeight * 2)

    setPos(w, 0)
end

function keydown(k, c)
    debug("keydown: " .. k .. "," .. c)
    local key = c
    -- result = ""
    if k == 13 then -- Enter
        exec(line)
        line = ""
    elseif k == 8 then -- Backspace
        line = subChar(line, 1, utf8.len(line))
    elseif k == 37 then -- ArrowLeft
    elseif k == 39 then -- ArrowRight
    elseif k == 38 then -- ArrowUp
    elseif k == 40 then -- ArrowDown
    elseif string.len(key) == 1 or utf8.len(key) == 1 then
     line = line .. key
    end
    draw(setPos)
end

function exec(line)
    result = ""
    local parts = split(line, " ")
    if parts[1] == "ls" then
        local files = getfiles()
        for i=1, #files do
            result = result .. files[i] .. " "
        end
    elseif parts[1] == "edit" then
        result = "edit:"
        if #parts > 1 then
            result = result .. parts[2]
        end
    elseif parts[1] == "run" then
        if #parts > 1 then
            run(parts[2])
        else
            result = "invalid argument"
        end
    end
end

require("skk")
imMode = M_HAN
draw(setPos)