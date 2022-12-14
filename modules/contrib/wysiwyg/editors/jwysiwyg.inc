<?php

/**
 * @file
 * Editor integration functions for jWYSIWYG.
 */

/**
 * Plugin implementation of hook_editor().
 */
function wysiwyg_jwysiwyg_editor() {
  $editor['jwysiwyg'] = array(
    'title' => 'jWYSIWYG',
    'vendor url' => 'https://github.com/jwysiwyg/jwysiwyg',
    'download url' => 'https://github.com/jwysiwyg/jwysiwyg/releases',
    'libraries' => array(
      '' => array(
        'title' => 'Source',
        'files' => array('jquery.wysiwyg.js'),
      ),
    ),
    'verified version range' => array('0.5', '0.97'),
    'deprecation message' => t('@editor is no longer being developed and support will be dropped by Wysiwyg module after May 2022.', array('@editor' => 'jWYSIWYG')),
    'install note callback' => 'wysiwyg_deprecation_install_note',
    'version callback' => 'wysiwyg_jwysiwyg_version',
    'settings form callback' => 'wysiwyg_jwysiwyg_settings_form',
    'settings callback' => 'wysiwyg_jwysiwyg_settings',
    // @todo Wrong property; add separate properties for editor requisites.
    'css path' => wysiwyg_get_path('jwysiwyg'),
    'versions' => array(
      '0.5' => array(
        'js files' => array('jwysiwyg.js'),
        'css files' => array('jquery.wysiwyg.css'),
      ),
    ),
  );
  return $editor;
}

/**
 * Enhances the editor profile settings form for jWYSIWYG.
 */
function wysiwyg_jwysiwyg_settings_form(&$form, &$form_state) {
  $form['buttons']['#access'] = FALSE;
  $form['basic']['language']['#access'] = FALSE;
  $form['css']['#access'] = FALSE;
}

/**
 * Return runtime editor settings for a given wysiwyg profile.
 *
 * @param $editor
 *   A processed hook_editor() array of editor properties.
 * @param $config
 *   An array containing wysiwyg editor profile settings.
 * @param $theme
 *   The name of a theme/GUI/skin to use.
 *
 * @return
 *   A settings array to be populated in
 *   Drupal.settings.wysiwyg.configs.{editor}
 */
function wysiwyg_jwysiwyg_settings($editor, $config, $theme) {
  $settings = array(
    'initialContent' => '',
  );
  return $settings;
}

/**
 * Detect editor version.
 *
 * @param $editor
 *   An array containing editor properties as returned from hook_editor().
 *
 * @return
 *   The installed editor version.
 */
function wysiwyg_jwysiwyg_version(&$editor) {
  $script = $editor['library path'] . '/jquery.wysiwyg.js';
  if (!file_exists($script)) {
    $script = $editor['library path'] . '/jwysiwyg/jquery.wysiwyg.js';
    if (!file_exists($script)) {
      return;
    }
    $editor['library path'] .= '/jwysiwyg';
    $editor['editor path'] .= '/jwysiwyg';
  }
  $script = fopen($script, 'r');
  fgets($script);
  $line = fgets($script);
  if (preg_match('@([0-9\.]+)\D*$@', $line, $version)) {
    fclose($script);
    return $version[1];
  }
  fclose($script);
}

