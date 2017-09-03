enemy
{
    walk_anim = swim
    death_anim = dead
    speed = 3.0
    collision_trigger = enemy_fish_trigger
    snap_to_collision_y = false
}

physics_loader enemy_fish_trigger
{
    create_physics_on_init = false
    physics = res/physics/characters.physics#enemy_trigger_base
}

${AnimationScale} = 0.2

sprite_animation swim
{
    scale = ${AnimationScale}
    spritesheet = res/spritesheets/enemy.ss
    spriteprefix = fish_swim__
    loop = true
    autostart = true
    fps = 1.5
}

sprite_animation dead
{
    scale = ${AnimationScale}
    spritesheet = res/spritesheets/enemy.ss
    spriteprefix = fish_dead
    fps = 0
}
