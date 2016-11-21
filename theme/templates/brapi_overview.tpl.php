<?php

/**
 * @file
 * Breeding API setting overview page.
 *
 * @ingroup brapi
 */
?>

<div class="brapi-admin-block">
<?php

  if (user_access(BRAPI_ADMIN_PERMISSION)
      || user_access('administer')) {
    print theme_table($overview_table);
  }

  if (user_access(BRAPI_USE_PERMISSION)
      || user_access(BRAPI_ADMIN_PERMISSION)
      || user_access('administer')) {
    print theme_table($call_table);
  }

  if (user_access(BRAPI_ADMIN_PERMISSION)
      || user_access('administer')) {
    print theme_table($cv_table);
  }

?>
</div>
