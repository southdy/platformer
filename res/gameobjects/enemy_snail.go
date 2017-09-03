enemy
{
    walk_anim = walk
    death_anim = dead
    speed = 1.5
    collision_trigger = enemy_snail_trigger
}

physics_loader enemy_snail_trigger
{
    create_physics_on_init = false
    physics = res/physics/characters.physics#enemy_trigger_base
}

${AnimationScale} = 0.2

sprite_animation walk
{
    scale = ${AnimationScale}
    spritesheet = res/spritesheets/enemy.ss
    spriteprefix = snail_walk__
    loop = true
    autostart = true
    fps = 3
}

sprite_animation dead
{
    scale = ${AnimationScale}
    spritesheet = res/spritesheets/enemy.ss
    spriteprefix = snail_dead
    fps = 0
}
