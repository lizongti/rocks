{
  "targets": [
    {
      "target_name": "stream_packet",
      "sources": [ "./src/v8.cpp", "./src/core.cpp"],
      'include_dirs': [
        '/usr/local/include',
      ],
      "cflags!": ["-std=gnu++17", "-O3", "-fno-exceptions"],
      "cflags_cc!": ["-std=gnu++17", "-O3", '-fno-exceptions'],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }]
      ]
    }
  ],
}