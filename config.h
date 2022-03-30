#ifndef __CONFIG_H__
#define __CONFIG_H__

#define title "Plane War"
#define screen_width 450
#define screen_height 700
#define game_fps 60

#define alive_score 10
#define alive_interval 1000
#define aircraft_down_interval 100
#define background_scroll_speed 1

#define p_enemy0 0.025
#define p_enemy1 0.015
#define p_enemy2 0.002

#define border_x 20
#define border_y 30
#define border_text 5

#define start 0
#define playing 1
#define pause 2
#define end 3

#define title_position 0.3
#define info_position 0.75
#define score_position 0.4
#define best_score_position 0.45

#define text_rect_width 300
#define text_rect_height 50
#define title_font_size 25
#define info_font_size 17

#define hero_width 50
#define hero_height 62
#define hero_speed 6
#define hero_hp 100
#define hero_status_max 5
#define hero_img_max 7
#define hero_change_interval 250
#define hero_fire_interval 100
#define hero_bullet_width 5
#define hero_bullet_height 10
#define hero_bullet_speed 10
#define hero_bullet_damage 100
#define hero_bomb_init_count 3

#define enemy0_width 25
#define enemy0_height 20
#define enemy0_speed 4
#define enemy0_hp 100
#define enemy0_score 100
#define enemy0_append_score 30
#define enemy0_status_max 5
#define enemy0_img_max 6

#define enemy1_width 40
#define enemy1_height 49
#define enemy1_speed 3
#define enemy1_hp 200
#define enemy1_score 1000
#define enemy1_status_max 5
#define enemy1_img_max 7
#define enemy1_append_score 1000
#define enemy1_fire_interval 400
#define enemy1_bullet_width 5
#define enemy1_bullet_height 10
#define enemy1_bullet_speed 7
#define enemy1_bullet_damage 40

#define enemy2_width 83
#define enemy2_height 128
#define enemy2_speed 2
#define enemy2_hp 1000
#define enemy2_score 10000
#define enemy2_status_max 7
#define enemy2_img_max 10
#define enemy2_append_score 10000
#define enemy2_change_interval 200
#define enemy2_fire_interval 800
#define enemy2_bullet_width 20
#define enemy2_bullet_height 20
#define enemy2_bullet_speed 15
#define enemy2_bullet_damage 200

#define hero_hit_dev 0
#define enemy_hit_dev 10
#define crash_dev 20

#define alive_status 0
#define down_status 1

#define appearance1 1
#define appearance2 2

#define color_black { 0,0,0 }
#define color_red { 255,0,0 }
#endif