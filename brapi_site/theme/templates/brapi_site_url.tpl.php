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
<span class="brapi-site-url">
    <a href="<?php print $brapi_url; ?>" title="<?php print check_plain($brapi_site->title) . ', API v' . check_plain($brapi_site->version); ?>">
      <?php print $brapi_url; ?>
    </a>
</span>
