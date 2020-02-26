# This file is part of Desktop App Toolkit,
# a set of libraries for developing nice desktop applications.
#
# For license and copyright information please follow this link:
# https://github.com/desktop-app/legal/blob/master/LEGAL

add_library(lib_wallet OBJECT)
add_library(desktop-app::lib_wallet ALIAS lib_wallet)
init_target(lib_wallet)

get_filename_component(src_loc . REALPATH)

set(style_files
    wallet/wallet.style
)

set(dependent_style_files
    ${submodules_loc}/lib_ui/ui/colors.palette
    ${submodules_loc}/lib_ui/ui/basic.style
    ${submodules_loc}/lib_ui/ui/layers/layers.style
    ${submodules_loc}/lib_ui/ui/widgets/widgets.style
)

generate_styles(lib_wallet ${src_loc} "${style_files}" "${dependent_style_files}")

set_target_properties(lib_wallet PROPERTIES AUTORCC ON)

target_precompile_headers(lib_wallet PRIVATE ${src_loc}/wallet/wallet_pch.h)
nice_target_sources(lib_wallet ${src_loc}
PRIVATE
    ${style_files}

    qrc/wallet.qrc

    ui/address_label.cpp
    ui/address_label.h
    ui/amount_label.cpp
    ui/amount_label.h
    ui/inline_diamond.cpp
    ui/inline_diamond.h
    ui/lottie_widget.cpp
    ui/lottie_widget.h
    ui/ton_word_input.cpp
    ui/ton_word_input.h
    ui/ton_word_suggestions.cpp
    ui/ton_word_suggestions.h
    wallet/create/wallet_create_check.cpp
    wallet/create/wallet_create_check.h
    wallet/create/wallet_create_created.cpp
    wallet/create/wallet_create_created.h
    wallet/create/wallet_create_import.cpp
    wallet/create/wallet_create_import.h
    wallet/create/wallet_create_intro.cpp
    wallet/create/wallet_create_intro.h
    wallet/create/wallet_create_manager.cpp
    wallet/create/wallet_create_manager.h
    wallet/create/wallet_create_passcode.cpp
    wallet/create/wallet_create_passcode.h
    wallet/create/wallet_create_ready.cpp
    wallet/create/wallet_create_ready.h
    wallet/create/wallet_create_step.cpp
    wallet/create/wallet_create_step.h
    wallet/create/wallet_create_view.cpp
    wallet/create/wallet_create_view.h
    wallet/wallet_change_passcode.cpp
    wallet/wallet_change_passcode.h
    wallet/wallet_common.cpp
    wallet/wallet_common.h
    wallet/wallet_confirm_transaction.cpp
    wallet/wallet_confirm_transaction.h
    wallet/wallet_cover.cpp
    wallet/wallet_cover.h
    wallet/wallet_create_invoice.cpp
    wallet/wallet_create_invoice.h
    wallet/wallet_delete.cpp
    wallet/wallet_delete.h
    wallet/wallet_empty_history.cpp
    wallet/wallet_empty_history.h
    wallet/wallet_enter_passcode.cpp
    wallet/wallet_enter_passcode.h
    wallet/wallet_export.cpp
    wallet/wallet_export.h
    wallet/wallet_history.cpp
    wallet/wallet_history.h
    wallet/wallet_info.cpp
    wallet/wallet_info.h
    wallet/wallet_invoice_qr.cpp
    wallet/wallet_invoice_qr.h
    wallet/wallet_log.cpp
    wallet/wallet_log.h
    wallet/wallet_phrases.cpp
    wallet/wallet_phrases.h
    wallet/wallet_receive_grams.cpp
    wallet/wallet_receive_grams.h
    wallet/wallet_send_grams.cpp
    wallet/wallet_send_grams.h
    wallet/wallet_sending_transaction.cpp
    wallet/wallet_sending_transaction.h
    wallet/wallet_settings.cpp
    wallet/wallet_settings.h
    wallet/wallet_top_bar.cpp
    wallet/wallet_top_bar.h
    wallet/wallet_update_info.cpp
    wallet/wallet_update_info.h
    wallet/wallet_view_transaction.cpp
    wallet/wallet_view_transaction.h
    wallet/wallet_window.cpp
    wallet/wallet_window.h
)

target_include_directories(lib_wallet
PUBLIC
    ${src_loc}
)

target_link_libraries(lib_wallet
PUBLIC
    desktop-app::lib_ton
    desktop-app::lib_ui
    desktop-app::lib_lottie
    desktop-app::lib_qr
)
