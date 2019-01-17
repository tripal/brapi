<?php

/**
 * @file
 * Breeding API comparator interface page.
 *
 * @ingroup brapi
 */
?>

<h3>Breeding API Comparator Interface</h3>

<form id="brapi_comparator_settings">
  <label><?php echo t('First BrAPI Service URL:'); ?> <input id="brapi_comparator_url1" name="brapi_comparator_url1" class="form-text" type="text" value="<?php echo $local_brapi_url;?>" size="60"/></label><br/>
  <label><?php echo t('Second BrAPI Service URL:'); ?> <input id="brapi_comparator_url2" name="brapi_comparator_url2" class="form-text" type="text" value="https://musabase.org/brapi/v1/" size="60"/></label><br/>
</form>

<div id="brapi_comparator">
  <form class="brapi-comparator" action="" method="post" accept-charset="UTF-8">
    <label>Entity type <select name="entity_type">
      <option value="germplasm" selected="selected">Germplasm</option>
    </select></label>
    <br/>
    <label>
      JSON list of entities to compare:<br/>
      <textarea id="brapi_data_list" name="brapi_data_list" class="form-textarea" cols="60" rows="5"></textarea>
    </label>
    <br/>
    <input class="brapi-comparator-button" type="button" value="<?php echo t('Compare'); ?>"/>
  </form>
</div>

<h4 id="brapi_comparator_result_title">Comparator results</h4>
<code id="brapi_comparator_results">
</code>
