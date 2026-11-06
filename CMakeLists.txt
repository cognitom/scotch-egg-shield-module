# windmill のキー処理を ZMK の app へ足す。zephyr_library に分けず app に直接入れるのは、
# ZMK のヘッダー (zmk/*.h) と include パスを app のものとしてそのまま使えるようにするため
if(CONFIG_ZMK_BEHAVIOR_WM_CHAR)
  # include/dt-bindings はキーマップ (devicetree) からも読む。そちらは zephyr/module.yml の
  # dts_root から見つかる
  target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
  target_sources(app PRIVATE
    src/behaviors/behavior_wm_char.c
    src/key_output.c
    src/output.c
  )
endif()
