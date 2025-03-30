{
  "targets": [
    {
      "target_name": "villib1",
      "sources": [
        "src/common.cpp",
        "src/plugins/cJSON.c",
        "src/plugins/cJSON.h",
        "src/plugins/CJsonObject.cpp",
        "src/plugins/CJsonObject.h",
        "src/plugins/httplib.h",
        "src/plugins/shared_memory.h",
        "src/plugins/WzSharememory.cpp",
        "src/plugins/WzSharememory.h",
        "src/apphandle.h",
        "src/apphandle.cpp",
        "src/vilssh.cpp",
        "src/ysftp.cpp",
        "src/ychannel.cpp",
        "src/vilsshconn.cpp",
        "src/main.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "libraries": [  ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
      },
      'msvs_settings': {
        'VCCLCompilerTool': { "ExceptionHandling": 1, 'AdditionalOptions': [ '-std:c++17' ] }
      },
      'conditions': [
        ['OS=="win"', {
          "target_name": "villib1",
          "include_dirs": [
            "<!@(node -p \"require('node-addon-api').include\")",
            "<(module_root_dir)/../ThirdPartyLibraries/windows/x86/include",
            "<(module_root_dir)/../ThirdPartyLibraries/common/include"
          ],
          "libraries": [
            '<(module_root_dir)/../ThirdPartyLibraries/windows/x86/lib/libcrypto',
            '<(module_root_dir)/../ThirdPartyLibraries/windows/x86/lib/libssl',
            '<(module_root_dir)/../ThirdPartyLibraries/windows/x86/lib/zlib',
            '<(module_root_dir)/../ThirdPartyLibraries/windows/x86/lib/cpprest_2_10',
            '<(module_root_dir)/../ThirdPartyLibraries/windows/x86/lib/ssh'
          ],
        }],
        ['OS=="linux"', {
          "target_name": "villib1",
          "include_dirs": [
            "<!@(node -p \"require('node-addon-api').include\")",
            "<(module_root_dir)/../ThirdPartyLibraries/linux/include",
            "<(module_root_dir)/../ThirdPartyLibraries/common/include"
          ],
          "libraries": [
            '<(module_root_dir)/../ThirdPartyLibraries/linux/lib/libcrypto.a',
            '<(module_root_dir)/../ThirdPartyLibraries/linux/lib/libssl.a',
            '<(module_root_dir)/../ThirdPartyLibraries/linux/lib/libz.a',
            '<(module_root_dir)/../ThirdPartyLibraries/linux/lib/libcpprest.a',
            '<(module_root_dir)/../ThirdPartyLibraries/linux/lib/libssh.a'
          ],
        }],
        ['OS=="mac"', {
          "target_name": "villib1",
          "include_dirs": [
            "<!@(node -p \"require('node-addon-api').include\")",
            "<(module_root_dir)/../ThirdPartyLibraries/mac/include",
            "<(module_root_dir)/../ThirdPartyLibraries/common/include"
          ],
          "libraries": [
            '<(module_root_dir)/../ThirdPartyLibraries/mac/lib/libcrypto.a',
            '<(module_root_dir)/../ThirdPartyLibraries/mac/lib/libssl.a',
            '<(module_root_dir)/../ThirdPartyLibraries/mac/lib/libz.a',
            '<(module_root_dir)/../ThirdPartyLibraries/mac/lib/libmbedtls.a',
            '<(module_root_dir)/../ThirdPartyLibraries/mac/lib/libmbedcrypto.a',
            '<(module_root_dir)/../ThirdPartyLibraries/mac/lib/libmbedx509.a',
            '<(module_root_dir)/../ThirdPartyLibraries/mac/lib/libcpprest.a',
            '<(module_root_dir)/../ThirdPartyLibraries/mac/lib/libssh.a'
          ],
        }]
      ]
    }
  ]
}