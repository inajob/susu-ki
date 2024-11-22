-- init
screenWidth = screenwidth()
screenHeight = screenheight()
fontHeight = 12

debug("== init ==")
--debug(getfiles())
--debug(readfile("test.txt"))

require("alert")
require("prompt")

Editor = {}
Editor.new = function()
    local obj = {}
    obj.lines = {}
    obj.lines[#obj.lines + 1] = {value="Hello World", dirty=true}
    obj.lines[#obj.lines + 1] = {value="日本語 テスト", dirty=true}
    obj.x = 1
    obj.y = 1
    obj.scrollY = 0
    obj.alldirty = true
    obj.getText = function(self)
        local ls = {}
        for i, l in pairs(self.lines) do
            ls[#ls + 1] = l.value
        end
        return table.concat(ls, "\n")
    end
    obj.loadText = function(self, text)
        local lines = {}
        self.lines = {}
        for line in text:gmatch("[^\n]+") do
            self.lines[#self.lines + 1] = {value = line, dirty = true}
        end
    end
    obj.draw = function(self, setPos)
        local px = 0 -- (px)
        local py = 0 -- (px)
        local cx = 0 -- cursor pos
        local cy = 0
        local offset = 10
        if self.alldirty then
            color(255,255,255)
            fillrect(0,0,screenWidth,screenHeight)
            self.alldirty = false
        end
        color(0,0,0)
        for i, l in pairs(self.lines) do
            if i < self.scrollY or i - self.scrollY > screenHeight/fontHeight then
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
            if i == #self.lines then
                fillrect(0,py + fontHeight,screenWidth,fontHeight)
            end
            -- left blue bar
            color(0,0,255)
            fillrect(0, py, 3, fontHeight)
            for p, c in utf8.codes(l["value"]) do
                local uc = utf8.char(c)
                if i == self.y and j == self.x then
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
            if i == self.y and j == self.x then
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
    obj.keydown = function(self, k, c, ctrl)
        debug("keydown: " .. k .. "," .. c)
        local key = c
        if k == 13 then -- Enter
            local line = self.lines[self.y]["value"]
            self.lines[self.y]["value"] = subChar(line, 1, self.x)
            self.lines[self.y]["dirty"] = true
            table.insert(self.lines, self.y + 1, {
                value = subChar(line, self.x, utf8.len(line) + 1),
                dirty=true
            })
            self.x = 1
            self.y = self.y + 1
        elseif k == 8 then -- Backspace
            local line = self.lines[self.y]["value"]
            self.lines[self.y]["dirty"] = true
            if self.x == 1 then
                if self.y > 1 then
                    local px = utf8.len(self.lines[self.y - 1]["value"]) + 1
                    self.lines[self.y - 1]["value"] = self.lines[self.y - 1]["value"] .. self.lines[self.y]["value"]
                    self.lines[self.y]["value"] = ""
                    table.remove(self.lines, self.y)
                    self.y = self.y - 1
                    self.x = px
                end
            else
                self.lines[self.y]["value"] = subChar(line, 1, self.x - 1) .. subChar(line, self.x, utf8.len(line) + 1)
                self.x = self.x - 1
            end
        elseif k == 37 then -- ArrowLeft
            if self.x > 1 then
                self.x = self.x - 1
            end
        elseif k == 39 then -- ArrowRight
            if self.x <= utf8.len(self.lines[self.y]["value"]) then
                self.x = self.x + 1
            end
        elseif k == 38 then -- ArrowUp
            if self.y > 1 then
                self.y = self.y - 1
                if self.x > utf8.len(self.lines[self.y]["value"]) + 1 then
                    self.x = utf8.len(self.lines[self.y]["value"]) + 1
                end
            end
        elseif k == 40 then -- ArrowDown
            if self.y < #self.lines then
                self.y = self.y + 1
                if self.x > utf8.len(self.lines[self.y]["value"]) + 1 then
                    self.x = utf8.len(self.lines[self.y]["value"]) + 1
                end
            end
        elseif key == "q" and ctrl then
            exit()
        elseif key == "l" and ctrl then
            local prompt = Prompt.new(
                "load...",
                function(fileName)
                    table.remove(windows)
                    local text = readfile(fileName)
                    if text == nil then
                        showAlert("Load Error! " .. fileName)
                    else
                        self:loadText(text)
                    end
                    local app = windows[#windows]
                    app.alldirty = true
                    app:draw()
                end,
                function()
                    table.remove(windows)
                    local app = windows[#windows]
                    app.alldirty = true
                    app:draw()
                end
            )
            windows[#windows + 1] = prompt
            prompt.draw(prompt, setPos)
            return
        elseif key == "s" and ctrl then
            showPrompt("Save...", function(fileName)
                local b = self:getText()
                    debug("SAVE" .. b)
                    savefile(fileName, b)
                    showAlert("SAVE to " .. fileName)
            end)
            return
        elseif key == "z" and ctrl then
            showAlert("てすと")
            return
        elseif string.len(key) == 1 or utf8.len(key) == 1 then
            local line = self.lines[self.y]
            self.lines[self.y]["value"] = insertChar(line["value"], self.x, key)
            self.x = self.x + 1
        end
    
        if cy <= 0  and self.scrollY > 0 then
            self.scrollY = self.scrollY - 1
            self.alldirty = true
        end
    
        if cy >= screenHeight - fontHeight * 2 then
            self.scrollY = self.scrollY + 1
            self.alldirty = true
        end
    
        self.draw(self, setPos)
    end
    return obj
end

-- utils
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

editor = Editor.new()
windows = {editor}

function showAlert(msg)
    local alert = Alert.new(msg, function()
        table.remove(windows)
        local app = windows[#windows]
        app.alldirty = true
        app:draw(setPos)
    end)
    windows[#windows + 1] = alert
    alert:draw()
end

function showPrompt(msg, handler)
    local prompt = Prompt.new(
        msg,
        function(text)
            table.remove(windows)
            handler(text)
        end,
        function()
            table.remove(windows)
            local app = windows[#windows]
            app.alldirty = true
            app:draw()
        end
    )
    windows[#windows + 1] = prompt
    prompt.draw(prompt, setPos)
end

function draw(setPos)
    for i, w in pairs(windows) do
        w:draw(setPos)
    end
end

function keydown(k, c, ctrl)
    local top = windows[#windows]
    top:keydown(k, c, ctrl)
end

draw()
require("skk")
-- ^ setPos defined here!!