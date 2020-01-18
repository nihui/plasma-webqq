find_package(Gettext REQUIRED)
file(GLOB POFILES "*.po")
gettext_create_translations(plasma_applet_webqq.pot ALL ${POFILES})
