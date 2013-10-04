geoip-udf
=========

MySQL UDF wrappers that call into the [MaxMind open source GeoIP C API](https://github.com/maxmind/geoip-api-c).

These user-defined functions were developed against MySQL 5.1.

Exposes the following SQL functions:

```sql
  GEOIP_COUNTRY_CODE(ipaddr)
  GEOIP_COUNTRY_NAME(ipaddr)
  GEOIP_REGION(ipaddr)
  GEOIP_CITY(ipaddr)
  GEOIP_DMA_CODE(ipaddr)
  GEOIP_AREA_CODE(ipaddr)
  GEOIP_LATITUDE(ipaddr)
  GEOIP_LONGITUDE(ipaddr)
  GEOIP_POSTAL_CODE(ipaddr)
  GEOIP_ORG(ipaddr)
  GEOIP_ISP(ipaddr)
```

License
--------
Released under the New BSD License

Copyright
-------------------
Copyright (c) 2010, Anchor Intelligence. All rights reserved.
