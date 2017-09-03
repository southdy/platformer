enemy
{
    walk_anim = walk
    death_anim = dead
    speed = 3.0
    collision_trigger = enemy_slime_trigger
}

physics_loader enemy_slime_trigger
{
    create_physics_on_init = false
    physics = res/physics/characters.physics#enemy_trigger_base
}

${AnimationScale} = 0.2

sprite_animation walk
{
    scale = ${AnimationScale}
    spritesheet = res/spritesheets/enemy.ss
    spriteprefix = slime_walk__
    loop = true
    autostart = true
    fps = 3
}

sprite_animation dead
{
    scale = ${AnimationScale}
    spritesheet = res/spritesheets/enemy.ss
    spriteprefix = slime_dead
    fps = 0
}
