{
    "targets": [ {
        "target_name": "database",
        "sources": [
            "src/database.cpp",
            "src/global.cpp",
            "src/graph.cpp",
            "src/IGTree.cpp",
            "src/save_load.cpp",
            "src/i_network.cpp",
            "src/i_cars.cpp"
        ],
        "include_dirs": [
            "include/"
        ],
        'link_settings': {
            'libraries': [
                # '-fsanitize=address',
                # '-static-libasan',
                # '-lasan',
                "-lmetis",
                # "-fexceptions",
            ]
        },
        'cflags': [
            "-Ofast",
            # '-g',
            # "-fexceptions",
            "-std=c++11",
            "-Wno-unused-result",
            "-Wno-sign-compare",
            "-Wno-unused-variable",
            # "-fsanitize=address"
        ]
    }
    ]
}