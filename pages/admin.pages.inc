<?php 

function zombies_admin_form() {
  $form['generate'] = array(
    '#type' => 'fieldset',
    '#title' => t('Generate Zombies'),
    '#collapsible' => FALSE,
  );
  
  $options = array(
    '10' => '10', 
    '100' => '100', 
    '1000' => '1,000', 
    '10000' => '10,000', 
    '100000' => '100,000', 
    '1000000' => '1,000,000',
  );
  $form['generate']['amount'] = array(
    '#type' => 'select',
    '#title' => t('Amount'),
    '#description' => t('Select the number of zombies to generate (per city).'),
    '#options' => $options,
  );
  
  $form['generate']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate!'),
  );
  
  return $form;
}


function zombies_admin_form_submit($form, &$form_state) {
  $zombie_amount = $form_state['values']['amount'];
  
  // Get all cities
  $city_count = db_result(db_query("SELECT COUNT(*) FROM {geo_data_us_cities}"));
  
  
  // 0.00899322030318217 degrees ~ 1 km
  // 0.449662168687844 degress ~ 50 km
  // 0.899324337375688 degrees ~ 100 km
  for ($i = 0; $i < $zombie_amount; $i++) {
    db_query("INSERT INTO {zombies} (geo)
    			SELECT ST_MakePoint(
    				ST_X(geo) + ((random() * 0.899324337375688) - 0.449662168687844), 
    				ST_Y(geo) + ((random() * 0.899324337375688) - 0.449662168687844)
    				) FROM {geo_data_us_cities}");
  }
  
  // For every city generate $zombie_amount zombies at random positions
  drupal_set_message(format_plural($city_count * $zombie_amount, 'Generated 1 zombie.', 'Generated @count zombies.'));
}


function zombies_find_closest() {
  $form['zip'] = array(
    '#type' => 'textfield',
    '#size' => 5,
    '#maxlength' => 5,
    '#title' => t('Zip'),
    '#description' => t('Enter the zip code to find the closest zombies.'),
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
  	'#value' => t('Closest Zombie'),
  );
  
  $form['density'] = array(
    '#type' => 'submit',
    '#value' => t('Zombie Forecast'),
  );
  
  return $form;
}

function zombies_find_closest_submit($form, &$form_state) {
  $v = $form_state['values'];
  $zip = $v['zip'];
  $result = drupal_http_request("http://local.yahooapis.com/MapsService/V1/geocode?appid=ae2b1fca515949e5d54fb22b8ed95575&zip=$zip&output=php");
  $data = unserialize($result->data);
  $vals = $data['ResultSet']['Result'];

  
  if ($v['op'] == 'Closest Zombie') {
    $nearest_zombie_distance = zombies_nearest($vals['Longitude'], $vals['Latitude']);
    
    $replace = array(
      '@distance' => number_format($nearest_zombie_distance, 2, '.', ','),
    );
    $message = t('The nearest zombie is @distance km away.', $replace);
    drupal_set_message($message, 'warning');
  }
  else {
    $density = zombies_density($vals['Longitude'], $vals['Latitude']);
    
  
    if ($density >= 500) {
      $forecast = t('Braaaiinnnnsss!');
    }
    else {
      $forecast = t('Clear');
    }
        
    
    $replace = array(
      '@forecast' => $forecast,
    );
    $message = t("Today the zombie forecast is: @forecast", $replace);
    drupal_set_message($message, 'warning');
  }
}


function zombies_list_states() {
  $result = db_query("SELECT ST_Area(ST_Transform(geo, 2163))/1000000 AS area, state 
  					  FROM {geo_data_us_states} 
  					  ORDER BY ST_Area(geo) DESC");

  $header = array(t('State'), t('Area'));
  $rows = array();
  while ($state = db_fetch_object($result)) {
    $rows[] = array(
      $state->state,
      number_format($state->area, 2, '.', ',') . ' km<sup>2</sup>',
    );
  }
  
  return theme('table', $header, $rows);
}