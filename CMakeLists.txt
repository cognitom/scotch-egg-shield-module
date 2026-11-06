# windmill のキー処理を ZMK の app へ足す。zephyr_library に分けず app に直接入れるのは、
# ZMK のヘッダー (zmk/*.h) と include パスを app のものとしてそのまま使えるようにするため
if(CONFIG_ZMK_WINDMILL)
  # include/dt-bindings はキーマップ (devicetree) からも読む。そちらは zephyr/module.yml の
  # dts_root から見つかる
  target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
  # behavior どうしで共有する処理。接続先ごとの設定、かな / 英数のベース、出力表と
  # 文字の送り方は、どの behavior からも引くので分けない
  target_sources(app PRIVATE
    src/host.c
    src/host_config.c
    src/key_output.c
    src/lang.c
    src/layer.c
    src/output.c
  )
endif()

if(CONFIG_ZMK_BEHAVIOR_WM_CHAR)
  target_sources(app PRIVATE src/behaviors/behavior_wm_char.c)
endif()

if(CONFIG_ZMK_BEHAVIOR_WM_HOST)
  target_sources(app PRIVATE src/behaviors/behavior_wm_host.c)
endif()

if(CONFIG_ZMK_BEHAVIOR_WM_LANG)
  target_sources(app PRIVATE src/behaviors/behavior_wm_lang.c)
endif()

if(CONFIG_ZMK_BEHAVIOR_WM_MOD)
  target_sources(app PRIVATE src/behaviors/behavior_wm_mod.c)
endif()

if(CONFIG_ZMK_BEHAVIOR_WM_ALPHA)
  target_sources(app PRIVATE src/behaviors/behavior_wm_alpha.c)
endif()

if(CONFIG_ZMK_BEHAVIOR_WM_MO)
  target_sources(app PRIVATE src/behaviors/behavior_wm_mo.c)
endif()

if(CONFIG_ZMK_BEHAVIOR_WM_OUT)
  target_sources(app PRIVATE src/behaviors/behavior_wm_out.c)
endif()
