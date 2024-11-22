Alert = {}
Alert.new = function(msg, handler)
    local obj = {}
    obj.msg = msg
    obj.handler = handler
    obj.draw = function(self)
        local top = fontHeight
        color(0,0,0)
        fillrect(0,top - 1,screenWidth, fontHeight*1 + 2)
        color(200,200,255)
        fillrect(1,top, screenWidth - 2, fontHeight*1)
        
        -- title
        color(0,0,0)        
        text(self.msg, 10, top)
    end
    obj.keydown = function(self, k, c, ctrl)
        local key = c
        if k == 13 then -- Enter
            self.handler()
            return
        elseif k == 27 then -- Esc
            table.remove(windows)
            local app = windows[#windows]
            app.alldirty = true
            app.draw(app, setPos)
            return
        end
        self.draw(self)
    end
    return obj
end