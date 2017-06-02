<?php

/**
 * @file
 * Displays a list of available BrAPI sites.
 *
 * @ingroup brapi_site
 */

$bs_table = $variables['bs_table'];
$pager = $variables['pager'];
if ($bs_table) {
?>
<div class="brapi_site-data-block-desc tripal-data-block-desc">
  This is the list of available BrAPI sites.
</div>
<br/>
<?php
  print $bs_table;
  print $pager;
}
else {
?>
  <div class="brapi_site-message">
    No BrAPI site reference found! Would you like to <?php
    echo l(t('create a new BrAPI site reference'), 'brapi_site/add'); ?>?<br/>
    <br/>
  </div>
<?php
}
?>
