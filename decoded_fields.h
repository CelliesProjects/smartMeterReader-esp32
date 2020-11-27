/**
 * Define the data we're interested in, as well as the datastructure to
 * hold the parsed data. This list shows all supported fields, remove
 * any fields you are not using from the below list to make the parsing
 * and printing code smaller.
 * Each template argument below results in a field of the same name.
 */
using decodedFields = ParsedData<
  /* FixedValue */ energy_delivered_tariff1,
  /* FixedValue */ energy_delivered_tariff2,
  /* String */ electricity_tariff,
  /* FixedValue */ power_delivered,
  /* TimestampedFixedValue */ gas_delivered
>;
