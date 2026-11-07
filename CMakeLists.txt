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

# 最下段の listener は、hold-tap より後、keymap より前に置く (behavior_wm_bottom.c 参照)。
# ZMK の listener は .event_subscription セクションの並びで呼ばれ、その並びはリンクする
# オブジェクトの順で決まる。このファイルは ZMK の app の CMakeLists.txt より先に読まれ、
# ここで target_sources すると ZMK のどのソースよりも前 (hold-tap より前) に並ぶ。
# そこで app の CMakeLists.txt を読み終えてから、ソースの並びの keymap.c の直前へ差し込む。
#
# この並べ替えは ZMK の app/CMakeLists.txt の作り (app に target_sources で src/keymap.c を
# 足し、hold-tap がそれより前に並ぶこと) に頼っている。ZMK を上げたら
# tests/bottom-row/ctrl-then-thumb-is-ctrl-space で並びを確かめること
if(CONFIG_ZMK_BEHAVIOR_WM_BOTTOM)
  function(wm_place_bottom_listener source)
    get_target_property(sources app SOURCES)
    set(placed FALSE)
    set(reordered)
    foreach(item IN LISTS sources)
      if(NOT placed AND item MATCHES "(^|/)src/keymap\\.c$")
        list(APPEND reordered ${source})
        set(placed TRUE)
      endif()
      list(APPEND reordered ${item})
    endforeach()
    if(NOT placed)
      message(FATAL_ERROR "ZMK の app のソースに src/keymap.c が無く、最下段の listener の置き場所が決まらない")
    endif()
    set_property(TARGET app PROPERTY SOURCES ${reordered})
  endfunction()
  # DEFER CALL の引数の変数は呼ぶときに展開される (そのときの CMAKE_CURRENT_LIST_DIR は
  # app のもの) ので、EVAL でいまの値を埋めておく
  set(wm_bottom_source ${CMAKE_CURRENT_LIST_DIR}/src/behaviors/behavior_wm_bottom.c)
  cmake_language(EVAL CODE "
    cmake_language(DEFER DIRECTORY [[${CMAKE_SOURCE_DIR}]]
      CALL wm_place_bottom_listener [[${wm_bottom_source}]])
  ")
endif()

if(CONFIG_ZMK_BEHAVIOR_WM_OUT)
  target_sources(app PRIVATE src/behaviors/behavior_wm_out.c)
endif()
