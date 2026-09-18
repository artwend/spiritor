
function Set (list)
    local set = {}
    for _, l in ipairs(list) do set[l] = true end
    return set
end

dots = Set {
    --Hatred
    3014508696043520,
    4035839034130432,
    1465219503095808,
    --Dirty Fighting
    4063687602077696,
    --Vengeance
    1169953386397696,
    --Letality
    1703989619982336,
    3465720780292096
}

player = {}

function turn_around(degree)
    PressKey("a")
    Sleep(degree * 6,666)
    ReleaseKey("a")
end

function OnEvent(event, arg)
    if (event == "LOG") then
        --print(arg);
        if (arg.action.id == 836045448945489) then
            player.name = arg.who.name
        end
        --print("player name", player.name, "dots", dots[arg.ability.id])
        if (arg.target.name == player.name and dots[arg.ability.id]) then
        --if (arg.target.name == player.name and arg.ability.id == 2519049858711552) then
            print("dot", arg.ability.id)
            PressKey("lctrl")
            Sleep(60)
            PressKey("7")
            Sleep(20)
            ReleaseKey("7")
            Sleep(30)
            ReleaseKey("lctrl")
        end
        --OutputDebugMessage("New log entry: %d", arg);
    end
    if (event == "KEY_PRESSED") then
        OutputDebugMessage("key %d pressed", arg)
        if (arg == 57) then
            PressKey("lctrl")
            PressKey("0")
            Sleep(10)
            ReleaseKey("0")
            ReleaseKey("lctrl")
        end
        if (IsModifierPressed("lalt") and arg == 3) then
            --PressKey("lshift")
            PressKey("3")
            Sleep(20)
            ReleaseKey("3")
            Sleep(120)
            --ReleaseKey("lshift")
        end
        if (arg == 45) then
            turn_around(360)
        end
    end
    --if (event == "KEY_RELEASED" and arg == 57) then
    --    OutputDebugMessage("key %d released", arg)
    --    ReleaseKey(arg)
    --    return true
    --end
    if (event == "MOUSE_BUTTON_PRESSED" and arg == 6) then
        -- Mouse Button 6 has been pressed
    end
    if (event == "MOUSE_BUTTON_RELEASED" and arg == 6) then
        -- Mouse Button 6 has been released
    end
end