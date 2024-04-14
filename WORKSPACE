load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "libgam",
    remote = "https://git.sr.ht/~bentglasstube/gam",
    commit = "f58f6fab2aedc2ac6b9ab376a8b5e73ec73411f0",
)

git_repository(
    name = "gamecontrollerdb",
    remote = "https://github.com/gabomdq/SDL_GameControllerDB",
    commit = "6f3c4edcb5a2e2ed090ca8af40d2c0f00dcd77f6",
    build_file = "@libgam//:BUILD.gamecontrollerdb",
)
