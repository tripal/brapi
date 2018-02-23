<?php

/**
 * @file
 * This file contains template page pre-processing functions.
 *
 * @ingroup brapi
 */

/**
 * Returns statistics about a given permission.
 *
 * @param string $permission
 *   A permission name.
 *
 * @return array
 *   Returns an array with the following keys:
 *   - 'count' (integer or 'any'): number of existing active users that share
 *     the given permission;
 *   - 'roles' (array of string): an array of role name (HTML format) that
 *     provide the given permission.
 *
 * @ingroup brapi
 */
function brapi_theme_compute_role_stats($permission) {
  // Get roles having the permission.
  $user_roles = user_roles(FALSE, $permission);
  // Count users having the roles.
  if (in_array('anonymous user', $user_roles)) {
    // "anonymous user", so anybody.
    $user_count = 'any';
  }
  elseif (in_array('authenticated user', $user_roles)) {
    // All authenticated users (but only active ones).
    $query = 'SELECT COUNT(u.uid)
      FROM {users} u
      WHERE u.status = 1;';
    $result = db_query($query);
    $user_count = $result->fetchCol()[0];
  }
  else {
    // Get users count.
    $query = 'SELECT COUNT(DISTINCT(ur.uid))
      FROM {users_roles} ur
        JOIN {role} r ON r.rid = ur.rid
      WHERE r.name IN (:roles);';
    $result = db_query($query, array(':roles' => $user_roles));
    $user_count = $result->fetchCol()[0];
  }

  // Change display of special roles.
  $user_roles = str_replace(
    array(
      'anonymous user',
      'authenticated user',
    ),
    array(
      '<span class="brapi-spe-role">anonymous user</span>',
      '<span class="brapi-spe-role">authenticated user</span>',
    ),
    $user_roles
  );
  sort($user_roles);

  return array('count' => $user_count, 'roles' => $user_roles);
}

/**
 * Prepares Breeding API setting overview table data before rendering.
 *
 * @param array $permission_stats
 *   Array containing permission statistics.
 *
 * @return array
 *   Overview table data.
 *
 * @ingroup brapi
 */
function brapi_theme_prepare_overview_table($permission_stats) {
  $use_permission    = $permission_stats['use_permission'];
  $update_permission = $permission_stats['update_permission'];
  $admin_permission  = $permission_stats['admin_permission'];

  $use_permission_list =
    implode(', ', $use_permission['roles'])
    . ' ('
    . format_plural($use_permission['count'], '@count user', '@count users')
    . ')';
  $update_permission_list =
    implode(', ', $update_permission['roles'])
    . ' ('
    . format_plural($update_permission['count'], '@count user', '@count users')
    . ')';
  $admin_permission_list =
    implode(', ', $admin_permission['roles'])
    . ' ('
    . format_plural($admin_permission['count'], '@count user', '@count users')
    . ')';

  $rows = array(
    // Version.
    array(
      'data' => array(
        array(
          'data' => t('BrAPI implementation <strong>version</strong>'),
          'class' => 'status-title',
        ),
        array(
          'data' => BRAPI_SERVICE_VERSION,
          'class' => 'status-value',
        ),
      ),
    ),
    // Permissions.
    array(
      'data' => array(
        array(
          'data' => t('Roles that can <strong>use</strong> BrAPI'),
          'class' => 'status-title',
        ),
        array(
          'data' => $use_permission_list,
          'class' => 'status-value',
        ),
      ),
    ),
    array(
      'data' => array(
        array(
          'data' => t('Roles that can <strong>update</strong> database using BrAPI'),
          'class' => 'status-title',
        ),
        array(
          'data' => $update_permission_list,
          'class' => 'status-value',
        ),
      ),
    ),
    array(
      'data' => array(
        array(
          'data' => t('Roles that can <strong>administrate</strong> BrAPI'),
          'class' => 'status-title',
        ),
        array(
          'data' => $admin_permission_list,
          'class' => 'status-value',
        ),
      ),
    ),
  );

  $overview_table = array(
    'header' => array(),
    'rows' => $rows,
    'attributes' => array('class' => array('system-status-report')),
    'caption' => t('Breeding API Settings Overview'),
    'sticky' => FALSE,
    'colgroups' => array(),
    'empty' => t('No setting found!'),
  );
  return $overview_table;
}

/**
 * Prepare calls table data before rendering.
 *
 * @return array
 *   Call table data.
 *
 * @ingroup brapi
 */
function brapi_theme_prepare_call_table($simplified = FALSE) {
  global $base_url;
  $rows = array();
  // Get settings.
  $brapi_example_settings = variable_get(BRAPI_EXAMPLE_SETTINGS, array());
  $aggregation_options = variable_get(BRAPI_AGGREGATION_OPTIONS, array());
  // Get the list of available BrAPI sites.
  $brapi_sites = array();
  $brapi_site_lookup = array();
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'brapi_site');
  $results = $query->execute();
  if (!empty($results)) {
    // Load data about each connection.
    $brapi_sites = entity_load('brapi_site', array_keys($results['brapi_site']));
    foreach ($brapi_sites as $bsid => $entity) {
      $brapi_site_lookup[$entity->machine_name] = $entity;
    }
  }


  // Process each call.
  foreach (brapi_get_calls() as $brapi_call => $brapi_call_data) {
    $call_url = '/' . $brapi_call;
    // Remove trailing slash if one and not root.
    if ((1 < strlen($call_url)) && ('/' == $call_url[strlen($call_url) - 1])) {
      $call_url = substr($call_url, 0, -1);
    }

    // Replace URL arguments if some.
    if (array_key_exists('arguments', $brapi_call_data)) {
      $example_url_parts = $call_url_parts = explode('/', $call_url);
      foreach ($brapi_call_data['arguments'] as $abs_arg_index => $argument) {
        // Substract the index of BRAPI_SERVICE_PREFIX_V1 parts.
        $arg_index = $abs_arg_index - 1;
        $call_url_parts[$arg_index] =
          '<span class="brapi-argument" title="type: '
          . $argument['type'] . '; '
          . htmlentities($argument['description'])
          . '">'
          . $argument['name']
          . ($argument['required'] ?
              '<span class="brapi-required">*</span>'
              : ''
            )
          . '</span>';
        if (array_key_exists($argument['name'], $brapi_example_settings)) {
          $example_url_parts[$arg_index] =
            $brapi_example_settings[$argument['name']];
        }
        else {
          $example_url_parts[$arg_index] = $argument['name'];
        }
      }
      $call_url = implode('/', $call_url_parts);
      $example_url = implode('/', $example_url_parts);
    }
    else {
      $example_url = $call_url;
    }

    if ($example_url) {
      $call_url_display =
        l(
          $call_url,
          BRAPI_SERVICE_PREFIX_V1 . $example_url,
          array('html' => TRUE)
        );
    }
    else {
      $call_url_display = $call_url;
    }

    // Features.
    $features = array();
    if (array_key_exists('features', $brapi_call_data)) {
      foreach ($brapi_call_data['features'] as $feature => $feat_desc) {
        $features[] =
          '<div class="brapi-feature"><span class="brapi-feature-name">'
          . $feature
          . '</span>: <span class="brapi-feature-value">'
          . $feat_desc
          . '</span></div>';
      }
    }
    
    // Version.
    $version = $brapi_call ? $brapi_call_data['active version'] : t('n/a');
    if (!$version) {
      if ($simplified) {
        // Skip.
        continue;
      }
      $version = t('Not provided');
    }
    else if ('proxy' == $version) {
      $version = t('Proxied');
    }

    // Aggregation.
    $used_sites = array();
    if ($brapi_call && isset($aggregation_options[$brapi_call])) {
      foreach ($aggregation_options[$brapi_call] as $brapi_site_mn) {
        if (isset($brapi_site_lookup[$brapi_site_mn])) {
          if ($simplified) {
            $used_sites[] =
              '<span class="brapi-site">'
              . $brapi_site_lookup[$brapi_site_mn]->title
              . '</span>';
          }
          else {
            $used_sites[] =
              '<span class="brapi-site" title="'
              . $brapi_site_lookup[$brapi_site_mn]->url
              . '/'
              . $brapi_call
              . '">'
              . $brapi_site_lookup[$brapi_site_mn]->title
              . '</span>';
          }
        }
        elseif ($brapi_site_mn) {
          $used_sites[] = t('Missing entry "%site"', array('%site' => $brapi_site_mn));
        }
      }
    }
    $aggregation = implode(', ', $used_sites);

    // Methods.
    if ($simplified) {
      $methods = 'READ';
      if (array_key_exists('PUT', $brapi_call_data['methods'])) {
        $methods = 'CREATE, READ, UPDATE';
      }
      elseif (array_key_exists('POST', $brapi_call_data['methods'])) {
        $methods = 'READ, FILTERING';
      }

      if (array_key_exists('DELETE', $brapi_call_data['methods'])) {
        $methods .= ', DELETE';
      }

      // handle special case of login
      if ('token' == $brapi_call) {
        $methods = 'LOGIN, LOGOUT';
      }
    }
    else {
      $methods = implode(', ', array_keys($brapi_call_data['methods']));
    }

    $rows[] = array(
      'data' => array(
        array('data' => $call_url_display),
        array('data' => $brapi_call_data['title']),
        array('data' => $version),
        array('data' => $aggregation),
        array('data' => $methods),
        array('data' => $brapi_call_data['description']),
        array('data' => implode("\n", $features)),
      ),
    );
  }

  $call_table = array(
    'header' => array(
      t(
        'Call URL (<span style="text-transform: none;">@service_url</span>)',
        array('@service_url' => $base_url . '/' . BRAPI_SERVICE_PREFIX_V1 . '/')
      ),
      t('Call name'),
      t('Version'),
      t('Proxy/aggregation'),
      t('Supported methods'),
      t('Description'),
      t('Features'),
    ),
    'rows' => $rows,
    'attributes' => array('class' => array('system-status-report')),
    'caption' => t('Supported Calls'),
    'sticky' => FALSE,
    'colgroups' => array(),
    'empty' => t('No supported call found!'),
  );

  return $call_table;
}

/**
 * Prepare CV table data before rendering.
 *
 * @return array
 *   CV table data.
 *
 * @ingroup brapi
 */
function brapi_theme_prepare_cv_table() {
  // CV term table.
  $rows = array();
  // Get current CV settings.
  $brapi_cv_settings = brapi_get_cv();
  foreach (brapi_get_cv_settings() as $brapi_cv_term => $cvterm_ids) {
    if (!is_array($cvterm_ids)) {
      $cvterm_ids = array($cvterm_ids);
    }

    $terms = array();
    foreach ($cvterm_ids as $cvterm_id) {
      $term = tripal_get_cvterm(array('cvterm_id' => $cvterm_id));
      if ($term) {
        $terms[] =
          '<span class="brapi-cvterm" title="'
          . htmlentities($term->definition)
          . '">'
          . $term->name
          . ' <small class="brapi-minor-details">(cv: '
          . $term->cv_id->name
          . ', id:'
          . $term->cvterm_id
          . ')</small></span>';
      }
      elseif ($cvterm_id) {
        $terms[] =
          '<i class="brapi-missing">'
          . t(
            'term ID "@cvterm_id" not found',
            array('@cvterm_id' => $term->cvterm_id)
          )
          . '</i>';
      }
      else {
        $terms[] =
          '<i class="brapi-missing">' . t('no term set') . '</i>';
      }
    }
    $overridable = ($term && $term->cv_id->name == BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV) ?
      'brapi-yes">' . t('yes')
      : 'brapi-no">' . t('no');
    $rows[] =
      array(
        $brapi_cv_term,
        implode(', ', $terms),
        '<span class="' . $overridable . "</span>",
      );
  }

  $cv_table = array(
    'header' => array(
      t('Breeding API Term'),
      t('Chado CVTerm'),
      t('MCPD'),
    ),
    'rows' => $rows,
    'attributes' => array('class' => array('system-status-report')),
    'caption' => t('Term Settings'),
    'sticky' => FALSE,
    'colgroups' => array(),
    'empty' => t('No CV term setting found!'),
  );
  return $cv_table;
}

/**
 * Implements hook_preprocess_HOOK().
 *
 * @ingroup brapi
 */
function brapi_preprocess_brapi_overview(&$variables) {

  drupal_add_css(drupal_get_path('module', 'brapi') . '/theme/css/brapi.css');

  $use_permission    = brapi_theme_compute_role_stats(BRAPI_USE_PERMISSION);
  $update_permission = brapi_theme_compute_role_stats(BRAPI_UPDATE_PERMISSION);
  $admin_permission  = brapi_theme_compute_role_stats(BRAPI_ADMIN_PERMISSION);

  $permission_stats = array(
    'use_permission'    => $use_permission,
    'update_permission' => $update_permission,
    'admin_permission'  => $admin_permission,
  );

  $variables['overview_table'] = brapi_theme_prepare_overview_table($permission_stats);
  $variables['call_table']     = brapi_theme_prepare_call_table();
  $variables['cv_table']       = brapi_theme_prepare_cv_table();
}

/**
 * Implements hook_preprocess_HOOK().
 *
 * @ingroup brapi
 */
function brapi_preprocess_brapi_public_overview(&$variables) {

  drupal_add_css(drupal_get_path('module', 'brapi') . '/theme/css/brapi.css');

  $use_permission = brapi_theme_compute_role_stats(BRAPI_USE_PERMISSION);
  $variables['call_table'] = brapi_theme_prepare_call_table(TRUE);

}

/**
 * Implements hook_preprocess_HOOK().
 *
 * @ingroup brapi
 */
function brapi_preprocess_brapi_help(&$variables) {

  drupal_add_css(drupal_get_path('module', 'brapi') . '/theme/css/brapi.css');

  $variables['call_table'] = brapi_theme_prepare_call_table(TRUE);
}

/**
 * Implements hook_preprocess_HOOK().
 *
 * @ingroup brapi
 */
function brapi_preprocess_brapi_query(&$variables) {
  global $base_url;

  drupal_add_library('system', 'ui.datepicker');
  drupal_add_css(drupal_get_path('module', 'brapi') . '/theme/css/brapi.css');
  drupal_add_js(array('brapi' => array('imagePath' => $base_url . '/' . drupal_get_path('module', 'brapi') . '/theme/images/')), 'setting');
  drupal_add_js(drupal_get_path('module', 'brapi') . '/theme/js/brapi.query.ui.js');

  $variables['local_brapi_url'] = $base_url . '/' . BRAPI_SERVICE_PREFIX_V1;

  $variables['calls'] = array();
  foreach (brapi_get_calls() as $brapi_call => $brapi_call_data) {
    $call_args = NULL;
    if (array_key_exists('arguments', $brapi_call_data)) {
      $call_args = $brapi_call_data['arguments'];
    }
    // Add '/' to root call.
    if ('' === $brapi_call) {
      $brapi_call = '/';
    }
    $variables['calls'][$brapi_call] = array(
      'arguments' => $call_args,
      'filters' => brapi_get_call_filters($brapi_call),
    );
  }
}

/**
 * Implements hook_preprocess_HOOK().
 *
 * @ingroup brapi
 */
function brapi_preprocess_brapi_comparator(&$variables) {
  global $base_url;

  drupal_add_css(drupal_get_path('module', 'brapi') . '/theme/css/brapi.css');
  drupal_add_js(array('brapi' => array('imagePath' => $base_url . '/' . drupal_get_path('module', 'brapi') . '/theme/images/')), 'setting');
  drupal_add_js(drupal_get_path('module', 'brapi') . '/theme/js/brapi.comparator.js');

  $variables['local_brapi_url'] = $base_url . '/' . BRAPI_SERVICE_PREFIX_V1;

}
