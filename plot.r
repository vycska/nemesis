rm(list=ls())

file <- readline("enter file: ")

size <- file.size(file)
blocks <- size/16;

f <- readBin(file, "raw", size, 1)

d <- data.frame(date=integer(blocks), ds18b20_temp=integer(blocks), bme280_temp=integer(blocks), ds3231_temp=integer(blocks), bme280_hum=integer(blocks), bme280_pres=integer(blocks), adc_bat=integer(blocks))

for(i in 0:(size/16-1)) {
   d$date[i+1] <- readBin(f[(i*16+1) : (i*16+4)], "integer", 1, 4, endian="little")
   d$ds18b20_temp[i+1] <- readBin(f[(i*16+5) : (i*16+6)], "integer", 1, 2, endian="little")
   d$bme280_temp[i+1] <- readBin(f[(i*16+7) : (i*16+8)], "integer", 1, 2, endian="little")
   d$ds3231_temp[i+1] <- readBin(f[(i*16+9) : (i*16+10)], "integer", 1, 2, endian="little")
   d$bme280_hum[i+1] <- readBin(f[(i*16+11) : (i*16+12)], "integer", 1, 2, endian="little")
   d$bme280_pres[i+1] <- readBin(f[(i*16+13) : (i*16+14)], "integer", 1, 2, endian="little")
   d$adc_bat[i+1] <- readBin(f[(i*16+15) : (i*16+16)], "integer", 1, 2, endian="little")
}

d$date <- as.POSIXct(d$date, origin="1970-01-01", tz="GMT")
d$ds18b20_temp <- d$ds18b20_temp / 100.0
d$bme280_temp <- d$bme280_temp / 100.0
d$ds3231_temp <- d$ds3231_temp / 100.0
d$bme280_hum <- d$bme280_hum / 10.0
d$bme280_pres <- d$bme280_pres / 10.0
d$adc_bat <- d$adc_bat / 4095 * 3.3 * 2

repeat {

print(unique(as.Date(d$date)))

print(table(as.Date(d$date)))

day <- as.POSIXct(readline("enter day: "),tz="GMT")
type <- readline("enter plot [1 - temp, 2 - hum, 3 - pres]: ")

if(!(as.Date(day,tz="GMT")%in%as.Date(d$date,tz="GMT")) || !(type==1 || type==2 || type==3)) break;

filter <- (as.Date(d$date,tz="GMT")==as.Date(day,tz="GMT"))
x <- d$date[filter]
l <- length(x)

if(type==1) {
   baly <- "temperatura, C"
   mily <- c(min(d$ds18b20_temp[filter],d$bme280_temp[filter],d$ds3231_temp[filter])-2, max(d$ds18b20_temp[filter],d$bme280_temp[filter],d$ds3231_temp[filter])+2)
} else if(type==2) {
   baly <- "santykine dregmen, %"
   mily <- c(min(d$bme280_hum[filter])-10, max(d$bme280_hum[filter])+10)
} else if(type==3) {
   baly <- "slegis, mmHg"
   mily <- c(min(d$bme280_pres[filter])-10, max(d$bme280_pres[filter])+10)
}

plot(0, 0, type="n", main=day, xlab="laikas", ylab=baly, xlim=c(day, day+86400), ylim=mily, xaxt="n")
axis(side=1, at=seq(day,day+86400,len=9), labels=c("00:00","03:00","06:00","09:00","12:00","15:00","18:00","21:00","24:00"))
k <- 1
for(i in 2:(l+1)) {
   if(x[i] != x[i-1]+60 || i==l+1) {
      if(type==1) {
         lines((d$date[filter])[k:(i-1)], (d$ds18b20_temp[filter])[k:(i-1)], lwd=2, col="red")
         lines((d$date[filter])[k:(i-1)], (d$bme280_temp[filter])[k:(i-1)], lwd=2, col="blue")
         lines((d$date[filter])[k:(i-1)], (d$ds3231_temp[filter])[k:(i-1)], lwd=2, col="green")
      } else if(type==2) {
         lines((d$date[filter])[k:(i-1)], (d$bme280_hum[filter])[k:(i-1)], lwd=2, col="blue")
      } else if(type==3) {
         lines((d$date[filter])[k:(i-1)], (d$bme280_pres[filter])[k:(i-1)], lwd=2, col="red")
      }
      abline(v=x[k], lty=2)
      abline(v=x[i-1], lty=2)
      k <- i
   }
}

if(type==1) {
   legend("bottomright", legend=c("ds18b20", "bme280", "ds3231"), col=c("red", "blue", "green"), lwd=2)
} else if(type==2) {
   legend("bottomright", legend="bme280", col="blue", lwd=2)
} else if(type==3) {
   legend("bottomright", legend="bme280", col="red", lwd=2)
}

} #repeat
