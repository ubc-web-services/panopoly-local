; Panopoly Widgets Makefile

api = 2
core = 7.x

; Panopoly - Contrib - Fields

projects[tablefield][version] = 3.6
projects[tablefield][subdir] = contrib
projects[tablefield][patch][3128030] = https://www.drupal.org/files/issues/2020-04-22/tablefield-header-orientation-3128030-5.patch
projects[tablefield][patch][3137640] = https://www.drupal.org/files/issues/2020-05-18/tablefield-7008-fix-3137640-2.patch

projects[simple_gmap][version] = 1.5
projects[simple_gmap][subdir] = contrib
projects[simple_gmap][patch][2902178] = https://www.drupal.org/files/issues/2021-06-09/simple_gmap-iframe-title-2902178-19.patch

; Panopoly - Contrib - Widgets

projects[menu_block][version] = 2.9
projects[menu_block][subdir] = contrib

; Panopoly - Contrib - Files & Media

projects[file_entity][version] = 2.36
projects[file_entity][subdir] = contrib

projects[media][version] = 2.28
projects[media][subdir] = contrib

projects[media_youtube][version] = 3.11
projects[media_youtube][subdir] = contrib

projects[media_vimeo][version] = 2.1
projects[media_vimeo][subdir] = contrib
projects[media_vimeo][patch][2446199] = https://www.drupal.org/files/issues/no_exception_handling-2446199-1.patch
projects[media_vimeo][patch][2913855] = https://www.drupal.org/files/issues/media_vimeo_https-2913855-3.patch
