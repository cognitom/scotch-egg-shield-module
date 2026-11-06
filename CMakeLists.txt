# windmill のキー処理を ZMK の app へ足す。zephyr_library に分けず app に直接入れるのは、
# ZMK のヘッダー (zmk/*.h) と include パスを app のものとしてそのまま使えるようにするため
if(CONFIG_ZMK_BEHAVIOR_WM_CHAR OR CONFIG_ZMK_BEHAVIOR_WM_HOST)
  # include/dt-bindings はキーマップ (devicetree) からも読む。そちらは zephyr/module.yml の
  # dts_root から見つかる
  target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
  # 接続先ごとの設定。文字キーの出力表も &wm_host もここを引く
  target_sources(app PRIVATE
    src/host.c
    src/host_config.c
  )
endif()

if(CONFIG_ZMK_BEHAVIOR_WM_CHAR)
  target_sources(app PRIVATE
    src/behaviors/behavior_wm_char.c
    src/key_output.c
    src/output.c
  )
endif()

if(CONFIG_ZMK_BEHAVIOR_WM_HOST)
  target_sources(app PRIVATE src/behaviors/behavior_wm_host.c)
endif()
