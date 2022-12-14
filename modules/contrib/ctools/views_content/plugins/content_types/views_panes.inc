<?php

/**
 * @file
 * Content type plugin to allow Views to be exposed as a display type,
 * leaving most of the configuration on the view.
 */

/**
 * Implements hook_ctools_content_types()
 */
function views_content_views_panes_ctools_content_types() {
  return array(
    'title' => t('View panes'),
    'admin settings' => 'views_content_admin_form',
    'all contexts' => TRUE,
  );
}

/**
 * Return all content types available.
 */
function views_content_views_panes_content_type_content_types($plugin) {
  $types = array();
  // It can be fairly intensive to calculate this, so let's cache this in the
  // cache_views table. The nice thing there is that if views ever change, that
  // table will always be cleared. Except for the occasional default view, so
  // we must use the Views caching functions in order to respect Views caching
  // settings.
  views_include('cache');
  $data = views_cache_get('views_content_panes', TRUE);
  if (!empty($data->data)) {
    $types = $data->data;
  }

  if (empty($types)) {
    $types = array();

    $views = views_get_all_views();

    foreach ($views as $view) {
      if (!empty($view->disabled)) {
        continue;
      }

      $view->init_display();

      foreach ($view->display as $id => $display) {
        if (empty($display->handler->panel_pane_display)) {
          continue;
        }
        $info = _views_content_panes_content_type($view, $display);
        if ($info) {
          $types[$view->name . '-' . $id] = $info;
        }
      }

      $view->destroy();
    }
    views_cache_set('views_content_panes', $types, TRUE);
  }

  return $types;
}

/**
 * Return a single content type.
 */
function views_content_views_panes_content_type_content_type($subtype, $plugin) {
  list($name, $display) = explode('-', $subtype);
  $view = views_get_view($name);
  if (empty($view)) {
    return;
  }

  $view->set_display($display);
  $retval = _views_content_panes_content_type($view, $view->display[$display]);

  $view->destroy();
  return $retval;
}

function _views_content_panes_content_type($view, $display) {
  // Ensure the handler is the right type, as Views will fall back to
  // the default display if something is broken:
  if (!is_a($display->handler, 'views_content_plugin_display_panel_pane')) {
    return;
  }

  $title = views_content_get_display_title($view, $display->id);

  $description = $display->handler->get_option('pane_description');
  if (!$description) {
    $description = $view->description;
  }

  $category = $display->handler->get_option('pane_category');
  if (!$category['name']) {
    $category['name'] = t('View panes');
  }

  $icon = 'icon_views_page.png';

  $contexts = array();

  $arguments = $display->handler->get_argument_input();
  ctools_include('views');
  foreach ($arguments as $argument) {
    $contexts[] = ctools_views_get_argument_context($argument);
  }

  $allow = $display->handler->get_option('allow');
  return array(
    'title' => $title,
    'icon' => $icon,
    'description' => filter_xss_admin($description),
    'required context' => $contexts,
    'category' => array($category['name'], $category['weight']),
    'no title override' => empty($allow['title_override']),
  );
}

/**
 * Output function for the 'views' content type.
 *
 * Outputs a view based on the module and delta supplied in the configuration.
 */
function views_content_views_panes_content_type_render($subtype, $conf, $panel_args, $contexts) {
  if (!is_array($contexts)) {
    $contexts = array($contexts);
  }

  list($name, $display) = explode('-', $subtype);
  $view = views_get_view($name);
  if (empty($view)) {
    return;
  }

  $view->set_display($display);
  views_content_views_panes_add_defaults($conf, $view);

  if (!$view->display_handler->access($GLOBALS['user']) || empty($view->display_handler->panel_pane_display)) {
    return;
  }

  $conf['pane_contexts'] = $contexts;
  $conf['panel_args'] = $panel_args;
  $view->display_handler->set_pane_conf($conf);

  $allow = $view->display_handler->get_option('allow');
  if ($allow['path_override'] && !empty($conf['path'])) {
    $view->override_path = $conf['path'];
  }
  elseif ($path = $view->display_handler->get_option('inherit_panels_path')) {
    if (drupal_is_front_page()) {
      $view->override_path = '';
    }
    else {
      $view->override_path = $_GET['q'];
    }
  }

  $block = new stdClass();
  $block->module = 'views';
  $block->delta = $view->name . '-' . $display;

  if (($allow['link_to_view'] && !empty($conf['link_to_view'])) ||
      (!$allow['link_to_view'] && $view->display_handler->get_option('link_to_view'))) {
    $block->title_link = $view->get_url();
  }

  $stored_feeds = drupal_add_feed();

  $block->content = $view->preview();
  if (empty($view->result) && !$view->display_handler->get_option('empty') && empty($view->style_plugin->definition['even empty'])) {
    return;
  }

  // Add contextual links to the output.
  $block = (array) $block;
  views_add_block_contextual_links($block, $view, $display, 'panel_pane');
  $block = (object) $block;

  $block->title = $view->get_title();

  if (empty($view->total_rows) || $view->total_rows <= $view->get_items_per_page()) {
    unset($block->more);
  }

  if ((!empty($allow['feed_icons']) && !empty($conf['feed_icons'])) ||
      (empty($allow['feed_icons']) && $view->display_handler->get_option('feed_icons'))) {
    $new_feeds = drupal_add_feed();
    if ($diff = array_diff(array_keys($new_feeds), array_keys($stored_feeds))) {
      foreach ($diff as $url) {
        $block->feeds[$url] = $new_feeds[$url];
      }
    }
  }

  return $block;
}

/**
 * Add defaults to view pane settings.
 * This helps cover us if $allow settings changed and there are no actual
 * settings for a particular item.
 */
function views_content_views_panes_add_defaults(&$conf, $view) {
  $pager = $view->display_handler->get_option('pager');

  if (empty($conf)) {
    $conf = array();
  }

  $conf += array(
    'link_to_view' => $view->display_handler->get_option('link_to_view'),
    'more_link' => $view->display_handler->get_option('use_more'),
    'more_text' => $view->display_handler->get_option('more_text'),
    'feed_icons' => FALSE,
    'use_pager' => $pager['type'] != 'none' && $pager['type'] != 'some',
    'pager_id' => isset($pager['options']['id']) ? $pager['options']['id'] : 0,
    'items_per_page' => !empty($pager['options']['items_per_page']) ? $pager['options']['items_per_page'] : 10,
    'offset' => !empty($pager['options']['offset']) ? $pager['options']['offset'] : 0,
    'path_override' => FALSE,
    'path' => $view->get_path(),
    'fields_override' => $view->display_handler->get_option('fields_override'),
  );
}

/**
 * Returns an edit form for a block.
 */
function views_content_views_panes_content_type_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];
  $contexts = $form_state['contexts'];
  // This allows older content to continue to work, where we used to embed
  // the display directly.
  list($name, $display_id) = explode('-', $form_state['subtype_name']);
  $view = views_get_view($name);

  if (empty($view)) {
    $form['markup'] = array('#markup' => t('Broken/missing/deleted view.'));
    return $form;
  }

  $view->set_display($display_id);

  // If it couldn't set the display and we got the default display instead,
  // fail.
  if ($view->current_display == 'default') {
    $form['markup'] = array('#markup' => t('Broken/missing/deleted view display.'));
    return $form;
  }

  $allow = $view->display_handler->get_option('allow');

  // Provide defaults for everything in order to prevent warnings.
  views_content_views_panes_add_defaults($conf, $view);

  $form['arguments']['#tree'] = TRUE;

  foreach ($view->display_handler->get_argument_input() as $id => $argument) {
    if ($argument['type'] == 'user') {
      $form['arguments'][$id] = array(
        '#type' => 'textfield',
        '#default_value' => isset($conf['arguments'][$id]) ? $conf['arguments'][$id] : '',
        '#description' => t('You may use keywords for substitutions.'),
        '#title' => check_plain($argument['label']),
      );
    }
  }
  if ($allow['link_to_view']) {
    $form['link_to_view'] = array(
      '#type' => 'checkbox',
      '#default_value' => isset($conf['link_to_view']) ? $conf['link_to_view'] : $view->display_handler->get_option('link_to_view'),
      '#title' => t('Link title to page'),
    );
  }
  if ($allow['more_link']) {
    $form['more_link'] = array(
      '#type' => 'checkbox',
      '#default_value' => isset($conf['more_link']) ? $conf['more_link'] : $view->display_handler->get_option('use_more'),
      '#title' => t('Provide a "more" link.'),
    );
  }

  if ($allow['more_text']) {
    $form['more_text'] = array(
      '#type' => 'textfield',
      '#default_value' => isset($conf['more_text']) ? $conf['more_text'] : '',
      '#title' => t('Override "more" text'),
      '#description' => t('Leave blank to use the default text: "@more"', array('@more' => $view->display_handler->use_more_text())),
    );
  }

  if (!empty($allow['feed_icons'])) {
    $form['feed_icons'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($conf['feed_icons']),
      '#title' => t('Display feed icons'),
    );
  }

  $view->init_style();
  if ($allow['fields_override'] && $view->style_plugin->uses_fields()) {
    $form['fields_override'] = array(
      '#type' => 'fieldset',
      '#title' => 'Fields to display',
      '#collapsible' => TRUE,
      '#tree' => TRUE,
    );
    foreach ($view->display_handler->get_handlers('field') as $field => $handler) {
      $title = $handler->ui_name();
      if ($handler->options['label']) {
        $title .= ' (' . check_plain($handler->options['label']) . ')';
      }

      $form['fields_override'][$field] = array(
        '#type' => 'checkbox',
        '#title' => $title,
        '#default_value' => isset($conf['fields_override'][$field]) ? $conf['fields_override'][$field] : !$handler->options['exclude'],
      );
    }
  }

  ctools_include('dependent');
  if ($allow['use_pager']) {
    $form['use_pager'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use pager'),
      '#default_value' => $conf['use_pager'],
      '#id' => 'use-pager-checkbox',
      '#prefix' => '<div class="container-inline">',
    );
    $form['pager_id'] = array(
      '#type' => 'textfield',
      '#default_value' => $conf['pager_id'],
      '#title' => t('Pager ID'),
      '#size' => 4,
      '#id' => 'use-pager-textfield',
      '#dependency' => array('use-pager-checkbox' => array(1)),
      '#suffix' => '</div>',
    );
  }
  if ($allow['items_per_page']) {
    $form['items_per_page'] = array(
      '#type' => 'textfield',
      '#default_value' => $conf['items_per_page'],
      '#title' => t('Num items'),
      '#size' => 4,
      '#description' => t('Select the number of items to display, or 0 to display all results.'),
    );
  }
  if ($allow['offset']) {
    $form['offset'] = array(
      '#type' => 'textfield',
      '#default_value' => $conf['offset'],
      '#title' => t('Offset'),
      '#size' => 4,
      '#description' => t('Enter the number of items to skip; enter 0 to skip no items.'),
    );
  }
  if ($allow['path_override']) {
    $form['path'] = array(
      '#type' => 'textfield',
      '#default_value' => isset($conf['path']) ? $conf['path'] : $view->get_path(),
      '#title' => t('Override path'),
      '#size' => 30,
      '#description' => t('If this is set, override the View URL path; this can sometimes be useful to set to the panel URL.'),
    );
    if (!empty($contexts)) {
      $form['path']['#description'] .= ' ' . t('You may use substitutions in this path.');

      $form['contexts'] = array(
        '#type' => 'fieldset',
        '#title' => t('Substitutions'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );

      $rows = array();
      foreach ($contexts as $context) {
        foreach (ctools_context_get_converters('%' . check_plain($context->keyword) . ':', $context) as $keyword => $title) {
          $rows[] = array(
            check_plain($keyword),
            t('@identifier: @title', array('@title' => $title, '@identifier' => $context->identifier)),
          );
        }
      }

      $header = array(t('Keyword'), t('Value'));
      $form['contexts']['context'] = array('#markup' => theme('table', array('header' => $header, 'rows' => $rows)));
    }
  }

  if (empty($conf['exposed'])) {
    $conf['exposed'] = array();
  }

  if ($allow['exposed_form']) {
    // If the exposed form is part of pane configuration, get the exposed
    // form re-tool it for our use.
    $exposed_form_state = array(
      'view' => &$view,
      'display' => &$view->display[$display_id],
    );

    $view->set_exposed_input($conf['exposed']);

    if (version_compare(views_api_version(), '3', '>=')) {
      $exposed_form_state['exposed_form_plugin'] = $view->display_handler->get_plugin('exposed_form');
    }
    $view->init_handlers();
    $exposed_form = array();
    $exposed_form = views_exposed_form($exposed_form, $exposed_form_state);

    $form['exposed'] = array(
      '#tree' => TRUE,
    );

    foreach ($exposed_form['#info'] as $id => $info) {
      $form['exposed'][$id] = array(
        '#type' => 'item',
        '#id' => 'views-exposed-pane',
      );

      if (!empty($info['label'])) {
        $form['exposed'][$id]['#title'] = $info['label'];
      }

      if (!empty($info['operator']) && !empty($exposed_form[$info['operator']])) {
        $form['exposed'][$id][$info['operator']] = $exposed_form[$info['operator']];
        $form['exposed'][$id][$info['operator']]['#parents'] = array('exposed', $info['operator']);
        $form['exposed'][$id][$info['operator']]['#default_value'] = isset($conf['exposed'][$info['operator']]) ? $conf['exposed'][$info['operator']] : '';
      }
      $form['exposed'][$id][$info['value']] = $exposed_form[$info['value']];
      $form['exposed'][$id][$info['value']]['#parents'] = array('exposed', $info['value']);
      $form['exposed'][$id][$info['value']]['#default_value'] = isset($conf['exposed'][$info['value']]) ? $conf['exposed'][$info['value']] : '';
    }
  }

  // The exposed sort stuff doesn't fall into $exposed_form['#info'] so we
  // have to handle it separately.
  if (isset($exposed_form['sort_by'])) {
    $form['exposed']['sort_by'] = $exposed_form['sort_by'];
  }

  if (isset($exposed_form['sort_order'])) {
    $form['exposed']['sort_order'] = $exposed_form['sort_order'];
  }

  // Add the view object to the form to allow additional customization.
  $form_state['view'] = $view;

  return $form;
}

/**
 * Store form values in $conf.
 */
function views_content_views_panes_content_type_edit_form_submit(&$form, &$form_state) {
  // Copy everything from our defaults.
  $keys = array('link_to_view', 'more_link', 'more_text', 'feed_icons', 'use_pager',
    'pager_id', 'items_per_page', 'offset', 'path_override', 'path', 'arguments', 'fields_override', 'exposed',
  );

  foreach ($keys as $key) {
    if (isset($form_state['values'][$key])) {
      $form_state['conf'][$key] = $form_state['values'][$key];
    }
  }
}

/**
 * Returns the administrative title for a type.
 */
function views_content_views_panes_content_type_admin_title($subtype, $conf, $contexts) {
  list($name, $display) = explode('-', $subtype);
  $view = views_get_view($name);
  if (empty($view) || empty($view->display[$display])) {
    return t('Deleted/missing view @view', array('@view' => $name));
  }

  $view->set_display($display);
  views_content_views_panes_add_defaults($conf, $view);

  $title = views_content_get_display_title($view, $display);

  return check_plain($title);
}

/**
 * Returns the administrative title for a type.
 */
function views_content_views_panes_content_type_admin_info($subtype, $conf, $contexts) {
  $info = array();

  list($view_name, $display_name) = explode('-', $subtype);
  $view = views_get_view($view_name);

  if (empty($view) || empty($view->display[$display_name])) {
    return;
  }

  $view->set_display($display_name);
  views_content_views_panes_add_defaults($conf, $view);

  // Add arguments first.
  if (!empty($conf['arguments'])) {
    $keys = array_keys($conf['arguments']);
    $values = array_values($conf['arguments']);
    $argument_input = $view->display_handler->get_option('argument_input');

    foreach ($conf['arguments'] as $key => $value) {
      if (!empty($value)) {
        $label = $argument_input[$key]['label'];
        $info[] = $label . ': ' . $value;
      }
    }
  }

  $block = new stdClass();
  if ($info) {
    $block->title = array_shift($info);

    $info[] = $view->display_handler->get_option('pane_description');
    $block->content = theme('item_list', array('items' => $info));
  }
  else {
    $block->title = $view->display_handler->get_option('pane_description');
    $block->content = '';
  }
  return $block;
}
