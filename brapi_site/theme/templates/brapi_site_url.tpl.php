<?php

/**
 * @file
 * A basic template for BrAPI site URL.
 *
 * Available variables:
 * - $brapi_site: BrAPI site entity.
 * - $brapi_url: The standard URL for viewing a BrAPI site entity.
 *
 * @ingroup brapi_site
 */
?>
<div class="brapi-site-url">
    <strong>URL:</strong>
    <?php
      print $brapi_url;
    ?>
</div>
