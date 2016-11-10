<?php

/**
 * @file
 * Breeding API administrative help page.
 *
 * @ingroup brapi
 */
?>

<h3>About Breeding API</h3>
<p>
The Breeding API specifies a standard interface for plant phenotype/genotype
databases to serve their data to crop breeding applications. It is a shared,
open API, to be used by all data providers and data consumers who wish to
participate. Initiated in May 2014, it is currently in an actively developing
state, so now is the time for potential participants to help shape the
specifications to ensure their needs are addressed. The listserve for
discussions and announcements is at <?php
print l(
  t('Cornell University'),
  'http://mail2.sgn.cornell.edu/cgi-bin/mailman/listinfo/plant-breeding-api'
);
?>. Additional documentation is in the <?php
print l(t('Github wiki'), 'https://github.com/plantbreeding/documentation');
?>.
</p>

<p>
The Breeding API Drupal implementation has been sponsored by <strong>Bioversity
International</strong>, a CGIAR Research Centre.<br/>
The Breeding API project has been sponsored by the <strong>Bill and Melinda
Gates Foundation</strong> which funded the breeding API hackathon in June 2015
in Seattle and in July 2016 in Ithaca.<br/>
<h4>Partners</h4>
<ul>
  <li><a href="http://www.bioversityinternational.org/" title="Bioversity International, a CGIAR Research Center">Bioversity International</a></li>
  <li><a href="http://www.integratedbreeding.net/breeding-management-system/" title="Breeding Management System from Integrated Breeding Platform">BMS</a></li>
  <li><a href="http://bti.cornell.edu/" title="Boyce Thompson Institute at Cornell University">BTI</a> (<a href="http://www.cassavabase.org/" title="Cassavabase">Cassavabase</a>, <a href="http://musabase.org/" title="Musabase">Musabase</a>)</li>
  <li><a href="http://www.cimmyt.org/" title="International Maize and Improvement Center">CIMMYT</a></li>
  <li><a href="http://cipotato.org/" title="International Potato Center">CIP</a></li>
  <li><a href="http://www.cirad.fr/" title="Centre de coop&eacute;ration internationale en recherche agronomique pour le d&eacute;veloppement">CIRAD</a></li>
  <li><a href="http://gobiiproject.org/" title="Genomic Open-source Breeding Informatics Initiative">GOBII Porject</a></li>
  <li><a href="http://irri.org/" title="International Rice Research Institute">IRRI</a></li>
  <li><a href="http://www.hutton.ac.uk/" title="James Hutton Institute">The James Hutton Institute</a></li>
  <li><a href="http://www.wur.nl/" title="Wageningen University &amp; Research">WUR</a></li>
</ul>
</p>

<h3>Setup Instructions</h3>
<ul>
  <li>You can select which Breeding API field correspond to which Chado CV term
  in your database instance using <?php
  print l(
    t('Breeding API settings page'),
    'admin/tripal/extension/brapi/configuration'
  );
  ?>
  </li>
  <li>
    You can configure access permission on the <?php
    print l(t('Drupal permission settings page'), 'admin/people/permissions');
    ?>.
  </li>
</ul>

<h3>API</h3>
<p>
  Currently the following features have been implemented:
  <dl class="brapi-dl clearfix">
<?php
  foreach ($call_table['rows'] as $row) {
    print "    <dt>" . $row['data'][0]['data'] . "</dt>\n";
    print "    <dd>" . $row['data'][1]['data'] . "</dd>\n";
  }
?>
  </dl>
</p>
