player
{
    idle_anim = idle
    walk_anim = walk
    cower_anim = cower
    jump_anim = jump
    climb_anim = climb
    swim_anim = swim
    speed = 9.0
    swim_speed_scale = 0.5
    jump_height = 2.1
    physics = physics
}

player_input
{
}

player_reset
{
}

physics_loader physics
{
    create_physics_on_init = false
    physics = res/physics/characters.physics#player_normal
}

${AnimationScale} = 0.2

sprite_animation walk
{
    spritesheet = res/spritesheets/player.ss
    spriteprefix = walk__
    loop = true
    scale = ${AnimationScale}
    fps = 8
}

sprite_animation climb : walk
{
    spriteprefix = climb__
    fps = 5
}

sprite_animation idle
{
    spritesheet = res/spritesheets/player.ss
    sprite = stand
    sprite = front
    fps = 0.25
    loop = true
    autostart = true
    scale = ${AnimationScale}
}

sprite_animation cower
{
    spritesheet = res/spritesheets/player.ss
    spriteprefix = hurt
    scale = ${AnimationScale}
}

sprite_animation jump
{
    spritesheet = res/spritesheets/player.ss
    sprite = jump
    sprite = hurt
    fps = 3
    scale = ${AnimationScale}
}

sprite_animation swim
{
    spritesheet = res/spritesheets/player.ss
    spriteprefix = swim__
    fps = 4
    loop = true
    autostart = true
    scale = ${AnimationScale}
}
