-- init
lines = {}
lines[#lines + 1] = {value="Hello World", dirty=true}
lines[#lines + 1] = {value="日本語 テスト", dirty=true}
x = 1 -- cursor x(row)
y = 1 -- cursor y(row)

scrollY = 0 -- scroll position in row
screenWidth = screenwidth()
screenHeight = screenheight()
fontHeight = 16
alldirty = true

debug("== init ==")
--debug(getfiles())
--debug(readfile("test.txt"))

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

function insertChar(s, i, t)
    local r = ""
    local counter = 1
    for p, c in utf8.codes(s) do
        if counter == i then
            r = r .. t
        end
        r = r .. utf8.char(c)
        counter = counter + 1
    end
    if counter == i then
        r = r .. t
    end
    return r
end

function draw(setPos)
    local px = 0 -- (px)
    local py = 0 -- (px)
    local cx = 0 -- cursor pos
    local cy = 0
    local offset = 10
    if alldirty then
        color(255,255,255)
        fillrect(0,0,screenWidth,480)
        alldirty = false
    end
    color(0,0,0)
    for i, l in pairs(lines) do
        if i < scrollY or i - scrollY > screenHeight/fontHeight then
            goto skip
        end
        px = 0
        local j = 1
        if l["dirty"] == false then
            goto continue
        end
        l["dirty"] = true
        color(255,255,255)
        fillrect(0,py,screenWidth,fontHeight)
        -- left blue bar
        color(0,0,255)
        fillrect(0, py, 3, fontHeight)
        for p, c in utf8.codes(l["value"]) do
            local uc = utf8.char(c)
            if i == y and j == x then
                -- cursor
                color(0,0,0)
                fillrect(offset + px, py, 1, fontHeight - 1)
                cx = px
                cy = py
            end
            if offset + px + textwidth(uc) > screenWidth then
                px = 0
                py = py + fontHeight
                color(255,255,255)
                fillrect(0,py,screenWidth,fontHeight)
            end
            color(0,0,0)
            text(uc, offset + px, py)
            px = px + textwidth(uc)
            j = j + 1
        end
        if i == y and j == x then
            -- draw cursor
            color(0,0,0)
            fillrect(offset + px, py, 1, fontHeight - 1)
            cx = px
            cy = py
        end
        ::continue::
        py = py + fontHeight
        ::skip::
    end

    if setPos then
        setPos(offset + cx, cy)
    end
end

draw()

function keydown(k, c, ctrl)
    debug("keydown: " .. k .. "," .. c)
    local key = c
    if k == 13 then -- Enter
        local line = lines[y]["value"]
        lines[y]["value"] = subChar(line, 1, x)
        lines[y]["dirty"] = true
        table.insert(lines, y + 1, {
            value = subChar(line, x, utf8.len(line) + 1),
            dirty=true
        })
        x = 1
        y = y + 1
    elseif k == 8 then -- Backspace
        local line = lines[y]["value"]
        lines[y]["dirty"] = true
        if x == 1 then
            if y > 1 then
            local px = utf8.len(lines[y - 1]["value"]) + 1
            lines[y - 1]["value"] = lines[y - 1]["value"] .. lines[y]["value"]
            lines[y]["value"] = ""
            table.remove(lines, y)
            y = y - 1
            x = px
            end
        else
            lines[y]["value"] = subChar(line, 1, x - 1) .. subChar(line, x, utf8.len(line) + 1)
            x = x - 1
        end
    elseif k == 37 then -- ArrowLeft
        if x > 1 then
            x = x - 1
        end
    elseif k == 39 then -- ArrowRight
        if x <= utf8.len(lines[y]["value"]) then
            x = x + 1
        end
    elseif k == 38 then -- ArrowUp
        if y > 1 then
            y = y - 1
            if x > utf8.len(lines[y]["value"]) + 1 then
                x = utf8.len(lines[y]["value"]) + 1
            end
        end
    elseif k == 40 then -- ArrowDown
        if y < #lines then
            y = y + 1
            if x > utf8.len(lines[y]["value"]) + 1 then
                x = utf8.len(lines[y]["value"]) + 1
            end
        end
    elseif key == "q" and ctrl then
        exit()
    elseif string.len(key) == 1 or utf8.len(key) == 1 then
        local line = lines[y]
        lines[y]["value"] = insertChar(line["value"], x, key)
        x = x + 1
    end

    if cy <= 0  and scrollY > 0 then
        scrollY = scrollY - 1
        alldirty = true
    end

    if cy >= screenHeight - fontHeight * 2 then
        scrollY = scrollY + 1
        alldirty = true
    end

    draw(setPos)
end

require("skk")