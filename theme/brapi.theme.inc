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
  // get roles having the permission
  $user_roles = user_roles(FALSE, $permission);
  // count users having the roles
  if (in_array('anonymous user', $user_roles)) {
    // anonymous, so anybody
    $user_count = 'any';
  }
  elseif (in_array('authenticated user', $user_roles)) {
    // all authenticated users (but only active ones)
    $query = 'SELECT COUNT(u.uid)
      FROM {users} u
      WHERE u.status = 1;';
    $result = db_query($query);
    $user_count = $result->fetchCol()[0];
  }
  else {
    // get users count
    $query = 'SELECT COUNT(DISTINCT(ur.uid))
      FROM {users_roles} ur
        JOIN {role} r ON r.rid = ur.rid
      WHERE r.name IN (:roles);';
    $result = db_query($query, array(':roles' => $user_roles));
    $user_count = $result->fetchCol()[0];
  }

  // change display of special roles
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

  return array('count' => $user_count, 'roles' => $user_roles, );
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

  $rows = array(
    // version
    array(
      'data' => array(
        array(
          'data' => t('BrAPI implementation <strong>version</strong>'),
          'class'=> 'status-title',
        ),
        array(
          'data' => BRAPI_SERVICE_VERSION,
          'class'=> 'status-value',
        ),
      ),
    ),
    // permissions
    array(
      'data' => array(
        array(
          'data' => t('Roles that can <strong>use</strong> BrAPI'),
          'class'=> 'status-title',
        ),
        array(
          'data' => implode(', ', $use_permission['roles'])
            . ' ('
            . $use_permission['count']
            . ' user'
            . (1 < $use_permission['count'] ? 's' : '')
            . ')',
          'class'=> 'status-value',
        ),
      ),
    ),
    array(
      'data' => array(
        array(
          'data' =>
            t('Roles that can <strong>update</strong> database using BrAPI'),
          'class'=> 'status-title',
        ),
        array(
          'data' => implode(', ', $update_permission['roles'])
            . ' ('
            . $update_permission['count']
            . ' user'
            . (1 < $update_permission['count'] ? 's' : '')
            . ')',
          'class'=> 'status-value',
        ),
      ),
    ),
    array(
      'data' => array(
        array(
          'data' => t('Roles that can <strong>administrate</strong> BrAPI'),
          'class'=> 'status-title',
        ),
        array(
          'data' => implode(', ', $admin_permission['roles'])
            . ' ('
            . $admin_permission['count']
            . ' user'
            . (1 < $admin_permission['count'] ? 's' : '')
            . ')',
          'class'=> 'status-value',
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
function brapi_theme_prepare_call_table() {
  global $base_url;
  $rows = array();
  // get settings
  $brapi_example_settings = variable_get(BRAPI_EXAMPLE_SETTINGS);
  // process each call
  foreach (brapi_get_calls() as $brapi_call => $brapi_call_data) {
    $call_url = $brapi_call;
    // remove trailing slash if one and not root
    if ((1 < strlen($call_url)) && ('/' == $call_url[strlen($call_url) - 1])) {
      $call_url = substr($call_url, 0, -1);
    }

    // replace URL arguments if some
    if (array_key_exists('arguments', $brapi_call_data)) {
      $example_url_parts = $call_url_parts = explode('/', $call_url);
      foreach ($brapi_call_data['arguments'] as $arg_index => $argument) {
        $call_url_parts[$arg_index] =
          '<span class="brapi-argument" title="type: '
          . $argument['type'] . '; '
          . htmlentities(t($argument['description']))
          . '">'
          . $argument['name']
          . ($argument['required'] ?
              '<span class="brapi-required">*</span>'
              : ''
            )
          . '</span>'
        ;
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
          array('html' => TRUE,)
        );
    }
    else {
      $call_url_display = $call_url;
    }

    // features
    $features = array();
    if (array_key_exists('features', $brapi_call_data)) {
      foreach ($brapi_call_data['features'] as $feature => $feat_desc) {
        $features[] =
          '<div class="brapi-feature"><span class="brapi-feature-name">'
          . $feature
          . '</span>: <span class="brapi-feature-value">'
          . $feat_desc
          . '</span></div>'
        ;
      }
    }

    $rows[] = array(
      'data' => array(
        array('data' => $call_url_display),
        array('data' => t($brapi_call_data['title'])),
        array('data' => implode(', ', array_keys($brapi_call_data['methods']))),
        array('data' => t($brapi_call_data['description'])),
        array('data' => implode("\n", $features)),
      ),
    );
  }

  $call_table = array(
    'header' => array(
      t(
        'Call URL (<span style="text-transform: none;">'
        . $base_url . '/' . BRAPI_SERVICE_PREFIX_V1
        . '/</span>)'
      ),
      t('Call name'),
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
  // CV term table
  $rows = array();
  // get current CV settings
  $brapi_cv_settings = brapi_get_cv();
  foreach (brapi_get_cv_settings() as $brapi_cv_term => $cvterm_ids) {
    if (!is_array($cvterm_ids))
    {
        $cvterm_ids = array($cvterm_ids);
    }

    $terms = array();
    foreach ($cvterm_ids as $cvterm_id) {
      if ($term = tripal_get_cvterm(array('cvterm_id' => $cvterm_id,))) {
        $terms[] =
          '<span class="brapi-cvterm" title="'
          . htmlentities(t($term->definition))
          . '">'
          . $term->name
          . ' <small class="brapi-minor-details">(cv: '
          . $term->cv_id->name
          . ', id:'
          . $term->cvterm_id
          . ')</small></span>'
        ;
      }
      else {
        $terms[] =
          t(
            '<i class="brapi-missing">term @cvterm_id not found</i>',
            array('@cvterm_id' => $term->cvterm_id,)
          );
      }
    }
    $rows[] =
      array(
        $brapi_cv_term,
        implode(', ', $terms),
        '<span class="'
          . (array_key_exists($brapi_cv_term, $brapi_cv_settings) ?
              'brapi-yes">'
                . l(t('yes'), 'admin/tripal/extension/brapi/configuration')
              : 'brapi-no">'
                . t('no')
            )
          . "</span>"
      );
  }

  $cv_table = array(
    'header' => array(
      t('Breeding API Term'),
      t('Chado CVTerm'),
      t('Overridable')
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

  drupal_add_css(drupal_get_path('module', 'brapi') .'/theme/css/brapi.css');

  // set the breadcrumb
  // $breadcrumb = array();
  // $breadcrumb[] = l('Home', '<front>');
  // $breadcrumb[] = l('Administration', 'admin');
  // $breadcrumb[] = l('Tripal', 'admin/tripal');
  // $breadcrumb[] = l('Extensions', 'admin/tripal/extensions');
  // $breadcrumb[] = l('Breeding API', 'admin/tripal/extensions/brapi');
  // drupal_set_breadcrumb($breadcrumb);
  $use_permission    = brapi_theme_compute_role_stats(BRAPI_USE_PERMISSION);
  $update_permission = brapi_theme_compute_role_stats(BRAPI_UPDATE_PERMISSION);
  $admin_permission  = brapi_theme_compute_role_stats(BRAPI_ADMIN_PERMISSION);

  $permission_stats = array(
    'use_permission'    => $use_permission,
    'update_permission' => $update_permission,
    'admin_permission'  => $admin_permission,
  );

  $variables['overview_table'] =
    brapi_theme_prepare_overview_table($permission_stats);
  $variables['call_table']     = brapi_theme_prepare_call_table();
  $variables['cv_table']       = brapi_theme_prepare_cv_table();
}

/**
 * Implements hook_preprocess_HOOK().
 *
 * @ingroup brapi
 */
function brapi_preprocess_brapi_help(&$variables) {

  drupal_add_css(drupal_get_path('module', 'brapi') .'/theme/css/brapi.css');

  // set the breadcrumb
  // $breadcrumb = array();
  // $breadcrumb[] = l('Home', '<front>');
  // $breadcrumb[] = l('Administration', 'admin');
  // $breadcrumb[] = l('Tripal', 'admin/tripal');
  // $breadcrumb[] = l('Extensions', 'admin/tripal/extensions');
  // $breadcrumb[] = l('Breeding API', 'admin/tripal/extensions/brapi');
  // drupal_set_breadcrumb($breadcrumb);
  $variables['call_table'] = brapi_theme_prepare_call_table();
}