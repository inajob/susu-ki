Prompt = {}
Prompt.new = function(msg, okHandler, cancelHandler)
    local obj = {}
    obj.line = ""
    obj.x = 1
    obj.msg = msg
    obj.okHandler = okHandler
    obj.cancelHandler = cancelHandler
    obj.draw = function(self, setPos)
        color(0,0,0)
        fillrect(0,0,screenWidth, fontHeight * 2)
        color(255,255,255)
        fillrect(1,1,screenWidth - 2, fontHeight * 2 - 2)
        color(200,200,255)
        fillrect(1, 1,screenWidth - 2, fontHeight * 1 - 2)
        
        -- title
        color(0,0,0)        
        text(self.msg, 10, 1)
        local offset = 2
        local j = 1
        local px = 0
        local py = fontHeight
        local cx = px
        local cy = py
        color(255, 255, 255)
        for p, c in utf8.codes(self.line) do
            local uc = utf8.char(c)
            if j == self.x then
                -- draw cursor
                color(0,0,0)
                fillrect(offset + px, py, 1, fontHeight - 1)
                cx = offset + px
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
        if j == self.x then
            -- draw cursor
            color(0,0,0)
            fillrect(offset + px, py, 1, fontHeight - 1)
            cx = offset + px
            cy = py
        end
        if setPos then
            setPos(cx, cy)
        end
    end
    obj.keydown = function(self, k, c, ctrl)
        local key = c
        if k == 13 then -- Enter
            self.okHandler(self.line)
            return
        elseif k == 27 then -- Esc
            self.cancelHandler()
            return
        elseif k == 8 then -- Backspace
            if self.x ~= 1 then
                self.line = subChar(self.line, 1, self.x - 1) .. subChar(self.line, self.x, utf8.len(self.line) + 1)
                self.x = self.x - 1
            end
        elseif k == 37 then -- ArrowLeft
            if self.x > 1 then
                self.x = self.x - 1
            end
        elseif k == 39 then -- ArrowRight
            if self.x <= utf8.len(self.line) then
                self.x = self.x + 1
            end
        elseif string.len(key) == 1 or utf8.len(key) == 1 then
            self.line = insertChar(self.line, self.x, key)
            self.x = self.x + 1
        end
        self.draw(self)
    end
    return obj
end