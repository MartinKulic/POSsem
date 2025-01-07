cmake_minimum_required(VERSION 3.22)
project(
  Hadik
  VERSION 0.0.5
  DESCRIPTION "Klasicka hra hadik pre jedneho alebo viacerych hracov. Cielom je zjest co najviac ovocia a co najviac narast"
  LANGUAGES C
)


add_subdirectory(libsll)
add_subdirectory(share)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(menu)
