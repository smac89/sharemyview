package ("qml-colors")
    set_homepage("https://gooroo.github.io/qml-colors/")
    set_description("Swiss-knife utility for using colors in QML")
    set_license("BSD-3-Clause")

    add_urls("https://github.com/GooRoo/qml-colors/archive/refs/tags/$(version).tar.gz")
    add_versions("v1.0", "c75295ec3997ea6cbf0221a91ba906e1013d6c1c399338aa7861fba0794396c1")

    -- TODO: Add "qt.qmlplugin" rule

    on_install(function (package)
        os.cp("qml", package:installdir())
        -- https://doc.qt.io/qt-5/qtqml-syntax-imports.html#qml-import-path
        package:addenv("QML2_IMPORT_PATH", path.join(package:installdir(), "qml"))
    end)
