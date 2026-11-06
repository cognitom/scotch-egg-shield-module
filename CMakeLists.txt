# windmill のキー処理を ZMK の app へ足す。zephyr_library に分けず app に直接入れるのは、
# ZMK のヘッダー (zmk/*.h) と include パスを app のものとしてそのまま使えるようにするため
target_sources_ifdef(CONFIG_ZMK_BEHAVIOR_WM_CHAR app PRIVATE src/behaviors/behavior_wm_char.c)
