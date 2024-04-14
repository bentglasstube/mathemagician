package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "summoner",
    data = ["//content"],
    linkopts = [
        "-lSDL2",
        "-lSDL2_image",
        "-lSDL2_mixer",
    ],
    srcs = ["main.cc"],
    deps = [
        "@libgam//:game",
        ":screens",
    ],
)

cc_library(
    name = "screens",
    srcs = [
        "dungeon_screen.cc",
        "title_screen.cc",
    ],
    hdrs = [
        "dungeon_screen.h",
        "title_screen.h",
    ],
    deps = [
        "@libgam//:backdrop",
        "@libgam//:screen",
        "@libgam//:text",
        ":camera",
        ":dungeon",
        ":hud",
    ],
)

cc_library(
    name = "config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    deps = ["@libgam//:game"],
)

cc_library(
    name = "camera",
    srcs = ["camera.cc"],
    hdrs = ["camera.h"],
    deps = [
        ":config",
        ":entities",
    ],
)

cc_library(
    name = "dungeon",
    srcs = ["dungeon.cc"],
    hdrs = ["dungeon.h"],
    deps = [
        "@libgam//:sprite",
        "@libgam//:graphics",
        "@libgam//:rect",
        "@libgam//:spritemap",
        "@libgam//:util",
        ":config",
        ":log",
        ":ui",
    ],
)

cc_library(
    name = "entities",
    srcs = [
        "entity.cc",
        "player.cc",
    ],
    hdrs = [
        "entity.h",
        "player.h",
    ],
    deps = [
        "@libgam//:graphics",
        "@libgam//:spritemap",
        "@libgam//:text",
        "@libgam//:util",
        ":config",
        ":dungeon",
    ],
)

cc_library(
    name = "log",
    hdrs = ["log.h"],
)

cc_library(
    name = "hud",
    srcs = ["hud.cc"],
    hdrs = ["hud.h"],
    deps = [
        "@libgam//:graphics",
        "@libgam//:spritemap",
        "@libgam//:text",
        ":config",
        ":dungeon",
        ":entities",
        ":ui",
    ],
)

cc_library(
    name = "ui",
    srcs = ["ui.cc"],
    hdrs = ["ui.h"],
    deps = [
        "@libgam//:graphics",
        "@libgam//:spritemap",
        ":config",
    ],
)
