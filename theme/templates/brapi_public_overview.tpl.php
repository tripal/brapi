<?php

/**
 * @file
 * Breeding API call public overview page.
 *
 * @ingroup brapi
 */
?>

<div class="brapi-overview-block">
<?php

  if (user_access(BRAPI_USE_PERMISSION)
      || user_access(BRAPI_ADMIN_PERMISSION)
      || user_access('administer')) {
    print theme_table($call_table);
  }

?>
</div>
