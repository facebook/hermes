// Copyright (C) 2022 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone
description: >
  TimeZone constructor accepts link names as its input.
features: [Temporal]
---*/

const testCases = [
  "Africa/Accra",  // Link Africa/Abidjan Africa/Accra        # Ghana
  "Africa/Bamako",  // Link Africa/Abidjan Africa/Bamako       # Mali
  "Africa/Banjul",  // Link Africa/Abidjan Africa/Banjul       # The Gambia
  "Africa/Conakry",  // Link Africa/Abidjan Africa/Conakry      # Guinea
  "Africa/Dakar",  // Link Africa/Abidjan Africa/Dakar        # Senegal
  "Africa/Freetown",  // Link Africa/Abidjan Africa/Freetown     # Sierra Leone
  "Africa/Lome",  // Link Africa/Abidjan Africa/Lome         # Togo
  "Africa/Nouakchott",  // Link Africa/Abidjan Africa/Nouakchott   # Mauritania
  "Africa/Ouagadougou",  // Link Africa/Abidjan Africa/Ouagadougou  # Burkina Faso
  "Atlantic/St_Helena",  // Link Africa/Abidjan Atlantic/St_Helena  # St Helena
  "Africa/Addis_Ababa",  // Link Africa/Nairobi Africa/Addis_Ababa   # Ethiopia
  "Africa/Asmara",  // Link Africa/Nairobi Africa/Asmara        # Eritrea
  "Africa/Dar_es_Salaam",  // Link Africa/Nairobi Africa/Dar_es_Salaam # Tanzania
  "Africa/Djibouti",  // Link Africa/Nairobi Africa/Djibouti
  "Africa/Kampala",  // Link Africa/Nairobi Africa/Kampala       # Uganda
  "Africa/Mogadishu",  // Link Africa/Nairobi Africa/Mogadishu     # Somalia
  "Indian/Antananarivo",  // Link Africa/Nairobi Indian/Antananarivo  # Madagascar
  "Indian/Comoro",  // Link Africa/Nairobi Indian/Comoro
  "Indian/Mayotte",  // Link Africa/Nairobi Indian/Mayotte
  "Africa/Blantyre",  // Link Africa/Maputo Africa/Blantyre      # Malawi
  "Africa/Bujumbura",  // Link Africa/Maputo Africa/Bujumbura     # Burundi
  "Africa/Gaborone",  // Link Africa/Maputo Africa/Gaborone      # Botswana
  "Africa/Harare",  // Link Africa/Maputo Africa/Harare        # Zimbabwe
  "Africa/Kigali",  // Link Africa/Maputo Africa/Kigali        # Rwanda
  "Africa/Lubumbashi",  // Link Africa/Maputo Africa/Lubumbashi    # E Dem. Rep. of Congo
  "Africa/Lusaka",  // Link Africa/Maputo Africa/Lusaka        # Zambia
  "Africa/Bangui",  // Link Africa/Lagos Africa/Bangui         # Central African Republic
  "Africa/Brazzaville",  // Link Africa/Lagos Africa/Brazzaville    # Rep. of the Congo
  "Africa/Douala",  // Link Africa/Lagos Africa/Douala         # Cameroon
  "Africa/Kinshasa",  // Link Africa/Lagos Africa/Kinshasa       # Dem. Rep. of the Congo (west)
  "Africa/Libreville",  // Link Africa/Lagos Africa/Libreville     # Gabon
  "Africa/Luanda",  // Link Africa/Lagos Africa/Luanda         # Angola
  "Africa/Malabo",  // Link Africa/Lagos Africa/Malabo         # Equatorial Guinea
  "Africa/Niamey",  // Link Africa/Lagos Africa/Niamey         # Niger
  "Africa/Porto-Novo",  // Link Africa/Lagos Africa/Porto-Novo     # Benin
  "Africa/Maseru",  // Link Africa/Johannesburg Africa/Maseru  # Lesotho
  "Africa/Mbabane",  // Link Africa/Johannesburg Africa/Mbabane # Eswatini
];

for (let id of testCases) {
  const tz = new Temporal.TimeZone(id);
  assert.sameValue(tz.id, id);
}
