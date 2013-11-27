
find_program(EDJE_CC_EXECUTABLE edje_cc)
if (NOT EDJE_CC_EXECUTABLE)
    message(FATAL_ERROR "edje_cc could not be found")
endif ()

set(THEME_BINARY_DIR "${CMAKE_BINARY_DIR}/data/themes")
file(MAKE_DIRECTORY ${THEME_BINARY_DIR})

set(ELBOW_THEME_OUTPUT ${THEME_BINARY_DIR}/elbow.edj)
set(ELBOW_THEME_SOURCE_DIR "${CMAKE_SOURCE_DIR}/data/themes")
set(ELBOW_THEME_SOURCE ${ELBOW_THEME_SOURCE_DIR}/elbow.edc)

add_custom_command(
    OUTPUT ${ELBOW_THEME_OUTPUT}
    COMMAND ${EDJE_CC_EXECUTABLE} -v -id ${ELBOW_THEME_SOURCE_DIR} ${ELBOW_THEME_SOURCE} ${ELBOW_THEME_OUTPUT}
    DEPENDS
        ${ELBOW_THEME_SOURCE}
)

add_custom_target(ELBOW_THEME ALL
    DEPENDS ${ELBOW_THEME_OUTPUT}
)

# Basic code was copied from webkit.org to customize theme
# http://trac.webkit.org/browser/trunk/Source/WebCore/platform/efl/DefaultTheme

set(WEBKIT_THEME_OUTPUT ${THEME_BINARY_DIR}/webkit.edj)

add_custom_command(
    OUTPUT ${WEBKIT_THEME_OUTPUT}
    COMMAND ${EDJE_CC_EXECUTABLE} -id widget/scrollbar webkit.edc ${WEBKIT_THEME_OUTPUT}
    DEPENDS
        webkit.edc
        widget/button/button.edc
        widget/button/img_button_focus.png
        widget/button/img_button_hover.png
        widget/button/img_button_normal.png
        widget/button/img_button_press.png
        widget/check/check.edc
        widget/check/img_check_off_focus.png
        widget/check/img_check_off_hover.png
        widget/check/img_check_off.png
        widget/check/img_check_on_focus.png
        widget/check/img_check_on_hover.png
        widget/check/img_check_on.png
        widget/combo/combo.edc
        widget/combo/combo_focus_button.png
        widget/combo/combo_focus_button_transparent.png
        widget/combo/combo_focus.png
        widget/combo/combo_focus_transparent.png
        widget/combo/combo_hover_button.png
        widget/combo/combo_hover_button_transparent.png
        widget/combo/combo_hover.png
        widget/combo/combo_hover_transparent.png
        widget/combo/combo_normal_button.png
        widget/combo/combo_normal_button_transparent.png
        widget/combo/combo_normal.png
        widget/combo/combo_normal_transparent.png
        widget/combo/combo_press_button.png
        widget/combo/combo_press_button_transparent.png
        widget/combo/combo_press.png
        widget/combo/combo_press_transparent.png
        widget/combo/icon.png
        widget/cursor/cursor.edc
        widget/entry/entry.edc
        widget/entry/img_focused.png
        widget/entry/img_hovered.png
        widget/entry/img_normal.png
        widget/file/file.edc
        widget/file/file_focus.png
        widget/file/file_hover.png
        widget/file/file_normal.png
        widget/file/file_press.png
        widget/mediacontrol/fullscreenbutton/enterfullscreenbutton.png
        widget/mediacontrol/fullscreenbutton/exitfullscreenbutton.png
        widget/mediacontrol/fullscreenbutton/fullscreen_button.edc
        widget/mediacontrol/mutebutton/mute_button.edc
        widget/mediacontrol/mutebutton/mutebutton.png
        widget/mediacontrol/mutebutton/unmutebutton.png
        widget/mediacontrol/playpausebutton/pausebutton.png
        widget/mediacontrol/playpausebutton/playbutton.png
        widget/mediacontrol/playpausebutton/playpause_button.edc
        widget/mediacontrol/seekbackwardbutton/seekbackward_button.edc
        widget/mediacontrol/seekbackwardbutton/seekbackwardbutton.png
        widget/mediacontrol/seekforwardbutton/seekforward_button.edc
        widget/mediacontrol/seekforwardbutton/seekforwardbutton.png
        widget/mediacontrol/togglecaptionsbutton/closedcaption_disabled.png
        widget/mediacontrol/togglecaptionsbutton/closedcaption.png
        widget/mediacontrol/togglecaptionsbutton/toggle_captions_button.edc
        widget/progressbar/bt_base.png
        widget/progressbar/progressbar.edc
        widget/progressbar/shelf_inset.png
        widget/radio/img_radio_off_focus.png
        widget/radio/img_radio_off_hover.png
        widget/radio/img_radio_off.png
        widget/radio/img_radio_on_focus.png
        widget/radio/img_radio_on_hover.png
        widget/radio/img_radio_on.png
        widget/radio/radio.edc
        widget/scrollbar/scrollbar.edc
        widget/search/cancel/cancel_normal_button2.png
        widget/search/cancel/cancel_normal_button.png
        widget/search/cancel/search_cancel.edc
        widget/search/decoration/decoration_normal_button.png
        widget/search/decoration/search_decoration.edc
        widget/search/field/field_focused.png
        widget/search/field/field_hovered.png
        widget/search/field/field_normal.png
        widget/search/field/search_field.edc
        widget/slider/slider.edc
        widget/slider/slider_fill_h.png
        widget/slider/slider_fill_v.png
        widget/slider/slider_h.png
        widget/slider/slider_thumb_h.png
        widget/slider/slider_thumb_press_h.png
        widget/slider/slider_thumb_press_v.png
        widget/slider/slider_thumb_v.png
        widget/slider/slider_v.png
        widget/spinner/sp_bg.png
        widget/spinner/sp_down_default.png
        widget/spinner/sp_down_hover.png
        widget/spinner/sp_down_pressed.png
        widget/spinner/spinner.edc
        widget/spinner/sp_up_default.png
        widget/spinner/sp_up_hover.png
        widget/spinner/sp_up_pressed.png
    WORKING_DIRECTORY ${ELBOW_THEME_SOURCE_DIR}
    VERBATIM
)

add_custom_target(WebKitTheme ALL
    DEPENDS ${WEBKIT_THEME_OUTPUT}
)

install(FILES "${WEBKIT_THEME_OUTPUT}" DESTINATION ${DATA_INSTALL_DIR}/themes)

