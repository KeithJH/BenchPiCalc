set(WARNING_OPTIONS
  /W4
  /permissive-
  /wd5030 # Allow unrecognized attributes, such as `gnu::target`
)

list(APPEND WARNING_OPTIONS /WX)
