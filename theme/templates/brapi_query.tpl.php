<?php

/**
 * @file
 * Breeding API query interface page.
 *
 * @ingroup brapi
 */

?>

<h3>Breeding API Query Interface</h3>

<form id="brapi_query_settings">
  <label><?php echo t('BrAPI Service URL:'); ?> <input id="brapi_query_url" name="brapi_query_url" type="text" value="<?php echo $local_brapi_url;?>"/></label>
  <?php
  if (1 < count($brapi_sites)) {
  ?>
  <div class="container-inline">
    <label for="brapi_site_urls">Available sites:</label>
    <select id="brapi_site_urls" name="brapi_site">
      <?php
      $selected = ' selected="selected"';
      foreach ($brapi_sites as $brapi_site_name => $brapi_url) {
        ?>
        <option value="<?php echo check_url($brapi_url); ?>"<?php echo $selected; ?>><?php echo check_plain($brapi_site_name); ?></option>
        <?php
        $selected = '';
      }
      ?>
    </select>
    
  </div>
  <?php
  }
  ?>
</form>

<div id="brapi_query_calls">
<h4>Calls</h4>
<?php
  foreach ($calls as $call_name => $call_info) {
    $brapi_query = drupal_get_form(
      'brapi_query_form',
      $call_name,
      $call_info
    );
    print render($brapi_query);
  }
?>
</div>

<h4 id="brapi_query_result_title">Service response</h4>
<code id="brapi_query_results">
</code>
