-- script/main.lua

local player = nil
local speed = 200.0

function Init()
    print("====================================")
    print("MIR Engine Lua Script Initialized!")
    print("====================================")

    local manager = Manager.Instance()

    -- 1. 플레이어 엔티티 생성
    player = manager:AddEntity()
    
    -- 2. Transform 컴포넌트 설정 (초기 위치: 640, 360 - 화면 중앙)
    Transform.SetPosition(player, 640.0, 360.0)
    
    -- 3. Rigidbody 컴포넌트 설정
    Rigidbody.SetVelocity(player, 0.0, 0.0)
    Rigidbody.SetGravity(player, 0.0) -- 중력 없음 (탑다운 뷰 예제)
    
    -- 4. Tag 지정
    Tag.Set(player, "Player")
    
    print("Player Entity Created: " .. tostring(player))
end

function Update(deltaTime)
    local vx = 0.0
    local vy = 0.0

    -- 키보드 입력 처리
    if Input.IsPressed(Key.A) or Input.IsPressed(Key.Left) then
        vx = -speed
    elseif Input.IsPressed(Key.D) or Input.IsPressed(Key.Right) then
        vx = speed
    end

    if Input.IsPressed(Key.W) or Input.IsPressed(Key.Up) then
        vy = -speed
    elseif Input.IsPressed(Key.S) or Input.IsPressed(Key.Down) then
        vy = speed
    end

    -- 속도 셋팅 및 이동 시스템 업데이트
    Rigidbody.SetVelocity(player, vx, vy)
    Movement.Update(player, deltaTime)

    -- 가끔 디버그 로그로 위치 출력 (1% 확률)
    if math.random(1, 100) == 1 then
        local px = Transform.GetPositionX(player)
        local py = Transform.GetPositionY(player)
        print(string.format("[Lua] Player Pos: (%.1f, %.1f) | Vel: (%.1f, %.1f)", px, py, vx, vy))
    end
end

function Shutdown()
    print("MIR Engine Lua Script Shutdown.")
end
