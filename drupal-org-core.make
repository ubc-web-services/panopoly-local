api = 2
core = 7.x

; Drupal Core
projects[drupal][type] = core
projects[drupal][version] = 7.91

; Fix for PHP 8.
projects[drupal][patch][3177541] = "https://www.drupal.org/files/issues/2022-06-02/3177541-7.x-reroll.patch"

