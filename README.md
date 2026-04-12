# HaQML Theme Engine

[![Qt6](https://img.shields.io/badge/Qt-6.10+-green.svg)](https://www.qt.io)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

A simple theme engine for Qt & QML that allows you to configure colors, spacing, corner rounding, and additional visual parameters - all in a single JSON file.

> [!WARNING]
> At the moment, the theme engine is still in an early and primitive stage of development, so I cannot guarantee that there won’t be errors during its use.

## Usage
### QML

First, you need to include `ThemeManager.h` in the project's `main.cpp`.
```c++
#include "ThemeEngine/ThemeManager.hpp"
```

After that, if you want to use themes globally, register a Singleton.
```c++
qmlRegisterSingletonType<ThemeManager>("HaThemeEngine", 1, 0, "Theme",
    [](QQmlEngine*, QJSEngine*) -> QObject * {
        auto* manager = ThemeManager::instance();
        manager->loadTheme(":/themes/Dark.json"); //The path of theme

        return manager;
    }
);
```


## Install to project

### Method 1: CMake + FetchContent
```cmake
include(FetchContent)

FetchContent_Declare(
        haqml_theme_engine
        GIT_REPOSITORY https://github.com/vvh7u7/haqml-theme-engine.git
        GIT_TAG v1.0.0
)

FetchContent_MakeAvailable(haqml-theme-engine)

target_link_libraries(your_app PRIVATE HaThemeEngine)
```

### Method 2: Git Submodule
```bash
git submodule add https://github.com/vvh7u7/haqml-theme-engine.git libs/haqml-theme-engine
```
