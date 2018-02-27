<?php

/**
 * @file
 * BrAPI site entity UI controller.
 */

/**
 * The UI controller for BrAPI site entities.
 */
class BrapiSiteUIController extends EntityDefaultUIController {

  /**
   * Overrides hook_menu().
   *
   * Overrides hook_menu() defaults. Main reason for doing this is that
   * parent class hook_menu() is optimized for entity type administration.
   */
  public function hook_menu() {

    $items = array();
    $id_count = 1;
    $wildcard = isset($this->entityInfo['admin ui']['menu wildcard']) ?
      $this->entityInfo['admin ui']['menu wildcard']
      : '%' . $this->entityType;

    $items[$this->path] = array(
      'title' => 'BrAPI site references',
      'description' => 'Add edit and update BrAPI sites.',
      'page callback' => 'theme',
      'page arguments' => array('brapi_site_list'),
      'type' => MENU_LOCAL_TASK,
      'access arguments' => array('access administration pages'),
      'file path' => drupal_get_path('module', 'system'),
      'file' => 'system.admin.inc',
    );

    // Change the add page menu to multiple types of entities.
    $items['brapi_site/add'] = array(
      'title' => 'Add a BrAPI site reference',
      'description' => 'Add a new reference to an existing BrAPI site.',
      'page callback'  => 'brapi_site_add_page',
      'access callback'  => 'brapi_site_access',
      'access arguments' => array('create'),
      'type' => MENU_SUGGESTED_ITEM,
      'weight' => 20,
      'file' => 'includes/brapi_site.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module']),
    );

    // Loading and editing BrAPI site entities.
    $items['brapi_site/' . $wildcard] = array(
      'page callback' => 'brapi_site_page_view',
      'page arguments' => array($id_count),
      'access callback' => 'brapi_site_access',
      'access arguments' => array('view', $id_count),
      'weight' => -10,
      'context' => MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE,
      'file' => 'includes/brapi_site.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module']),
    );

    $items['brapi_site/' . $wildcard . '/view'] = array(
      'title' => 'View',
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'weight' => -10,
      'context' => MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE,
    );

    $items['brapi_site/' . $wildcard . '/edit'] = array(
      'title' => 'Edit',
      'page callback' => 'brapi_site_form_wrapper',
      'page arguments' => array($id_count),
      'access callback' => 'brapi_site_access',
      'access arguments' => array('edit', $id_count),
      'file' => 'includes/brapi_site.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module']),
      'type' => MENU_LOCAL_TASK,
      'weight' => 0,
      'context' => MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE,
    );

    $items['brapi_site/' . $wildcard . '/delete'] = array(
      'title' => 'Delete',
      'page callback' => 'brapi_site_delete_form_wrapper',
      'page arguments' => array($id_count),
      'access callback' => 'brapi_site_access',
      'access arguments' => array('edit', $id_count),
      'type' => MENU_LOCAL_TASK,
      'context' => MENU_CONTEXT_INLINE,
      'weight' => 10,
      'file' => 'includes/brapi_site.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module']),
    );

    return $items;
  }

  /**
   * Renders a BrAPI site creation page.
   *
   * Create the markup for the add BrAPI site Entities page within the class
   * so it can easily be extended/overriden.
   */
  public function addPage() {
    $item = menu_get_item();
    $content = system_admin_menu_block($item);

    if (count($content) == 1) {
      $item = array_shift($content);
      drupal_goto($item['href']);
    }

    // Create a new empty entity.
    $brapi_site = entity_create(
      'brapi_site',
      array(
        'bsid' => 0,
        'title' => '',
        'machine_name' => '',
        'description' => '',
        'weight' => 0,
        'url' => '',
        'version' => '1.0',
        'public' => 0,
        'username' => '',
        'password' => '',
        'type' => 'brapi_site',
      )
    );

    return brapi_site_form_wrapper($brapi_site);
  }

  /**
   * Returns an overview table of available BrAPI sites.
   *
   * @return string
   *   A rendered (HTML) BrAPI sites overview table.
   */
  public function overviewTable($conditions = array()) {
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', $this->entityType);

    // Add all conditions to query.
    foreach ($conditions as $key => $value) {
      $query->propertyCondition($key, $value);
    }

    if ($this->overviewPagerLimit) {
      $query->pager($this->overviewPagerLimit);
    }
    $results = $query->execute();

    if (!empty($results)) {
      // Load data about each connection.
      $entities = entity_load('brapi_site', array_keys($results['brapi_site']));

      // Create the table.
      $rows = array();
      foreach ($entities as $bsid => $entity) {

        $title = l($entity->title, 'brapi_site/' . $entity->bsid);
        $operations =
          '<ul class="links inline"><li class="edit first">'
          . l(t('edit'), 'brapi_site/' . $entity->bsid . '/edit')
          . '</li><li class="delete last">'
          . l(t('delete'), 'brapi_site/' . $entity->bsid . '/delete')
          . '</li></ul>';
        $rows[$bsid] = array(
          'data' => array(
            'weight'         => $entity->weight,
            'title'          => $title,
            'description'    => $entity->description,
            'url'            => $entity->url,
            'version'        => $entity->version,
            'public'         => ($entity->public ? t('yes') : t('no')),
            'authentication' => (!empty($entity->username) || !empty($entity->password)) ? t('yes') : t('no'),
            'operations'     => $operations,
          ),
          'class' => array('draggable'),
        );
      }

      // To be added later:
      // drupal_add_tabledrag('brapi_site_table', 'order', 'sibling', 'weight').
      return theme(
        'table',
        array(
          'header' =>
            array(
              'weight'         => t('#'),
              'title'          => t('Label'),
              'description'    => t('Description'),
              'url'            => t('Service URL'),
              'version'        => t('API Version'),
              'public'         => t('Public site'),
              'authentication' => t('Has credentials'),
              'operations'     => t('Operations'),
            ),
          'rows' => $rows,
          'attributes' => array(
            'class' => array('brapi-site-list'),
            'id' => 'brapi_site_table',
          ),
        )
      );
    }
    else {
      return '';
    }
  }

}
