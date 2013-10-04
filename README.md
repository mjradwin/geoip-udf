geoip-udf
=========

MySQL UDF wrappers that call into the [MaxMind open source GeoIP C API](http://www.maxmind.com/app/c). Exposes the following functions:

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

License & copyright
-------------------
Copyright (c) 2010, Anchor Intelligence. All rights reserved.

Released under the BSD License
