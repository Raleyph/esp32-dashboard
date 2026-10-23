idf_component_register(
    SRCS
        "src/WebServer.cpp"

    INCLUDE_DIRS
        "include"

    EMBED_FILES
        "www/index.html"

        "www/styles/style.css"
        "www/styles/ui.css"
        "www/styles/tabs.css"

        "www/scripts/tabs.js"
        "www/scripts/fetch.js"
        "www/scripts/update.js"
        "www/scripts/dom.js"
        "www/scripts/formatters.js"
        "www/scripts/chart.js"
        "www/scripts/logs.js"

    REQUIRES
        esp_http_server
        diagnostics
		utils
		logging

    PRIV_REQUIRES
        espressif__cjson
)