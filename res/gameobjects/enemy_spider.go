enemy
{
    walk_anim = walk
    death_anim = dead
    speed = 1.5
    collision_trigger = enemy_spider_trigger
}

physics_loader enemy_spider_trigger
{
    create_physics_on_init = false
    physics = res/physics/characters.physics#enemy_trigger_base
}

${AnimationScale} = 0.2

sprite_animation walk
{
    scale = ${AnimationScale}
    spritesheet = res/spritesheets/enemy.ss
    spriteprefix = spider_walk__
    loop = true
    autostart = true
    fps = 8
}

sprite_animation dead
{
    scale = ${AnimationScale}
    spritesheet = res/spritesheets/enemy.ss
    spriteprefix = spider_dead
    fps = 0
}
