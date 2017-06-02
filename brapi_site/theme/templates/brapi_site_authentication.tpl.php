<?php

/**
 * @file
 * A basic template for BrAPI site URL.
 *
 * Available variables:
 * - $brapi_site: BrAPI site entity.
 *
 * @ingroup brapi_site
 */

if (isset($brapi_site->username) || isset($brapi_site->password)) {
?>

<div class="brapi-site-authentication">
<?php
  if (isset($brapi_site->username)) {
?>
  <div class="brapi-site-user">
    <strong>User name:</strong>
    <?php
      print $brapi_site->username;
    ?>
    <br/>
  </div>
  <?php
  }

  if (isset($brapi_site->password)) {
  ?>
  <div class="brapi-site-password">
    <strong>Password:</strong>
    <?php
      print preg_replace('/./', '*', $brapi_site->password);
    ?>
    <br/>
  </div>
  <?php
  }
  ?>
</div>

<?php
}
?>
