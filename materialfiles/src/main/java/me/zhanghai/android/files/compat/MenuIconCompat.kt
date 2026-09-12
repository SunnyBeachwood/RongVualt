package me.zhanghai.android.files.compat

import android.view.Menu
import androidx.appcompat.view.menu.MenuBuilder
import androidx.appcompat.widget.PopupMenu

/** Keeps semantic icons visible in toolbar overflow menus and every nested submenu. */
fun Menu.forceShowIconsCompat() {
    (this as? MenuBuilder)?.setOptionalIconsVisible(true)
    for (index in 0 until size()) getItem(index).subMenu?.forceShowIconsCompat()
}

fun PopupMenu.forceShowIconsCompat() {
    setForceShowIcon(true)
    menu.forceShowIconsCompat()
}
