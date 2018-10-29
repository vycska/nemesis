rm(list=ls())

file <- readline("enter file: ")

size <- file.size(file)
blocks <- size/16;

f <- readBin(file, "raw", size, 1)

df <- data.frame(date=integer(blocks), ds18b20_temp=integer(blocks), bme280_temp=integer(blocks), ds3231_temp=integer(blocks), bme280_hum=integer(blocks), bme280_pres=integer(blocks), adc_bat=integer(blocks))

for(i in 0:(size/16-1)) {
   df$date[i+1] <- readBin(f[(i*16+1) : (i*16+4)], "integer", 1, 4, endian="little")
   df$ds18b20_temp[i+1] <- readBin(f[(i*16+5) : (i*16+6)], "integer", 1, 2, endian="little")
   df$bme280_temp[i+1] <- readBin(f[(i*16+7) : (i*16+8)], "integer", 1, 2, endian="little")
   df$ds3231_temp[i+1] <- readBin(f[(i*16+9) : (i*16+10)], "integer", 1, 2, endian="little")
   df$bme280_hum[i+1] <- readBin(f[(i*16+11) : (i*16+12)], "integer", 1, 2, endian="little")
   df$bme280_pres[i+1] <- readBin(f[(i*16+13) : (i*16+14)], "integer", 1, 2, endian="little")
   df$adc_bat[i+1] <- readBin(f[(i*16+15) : (i*16+16)], "integer", 1, 2, endian="little")
}

df$date <- as.POSIXct(df$date, origin="1970-01-01", tz="GMT")
df$ds18b20_temp <- df$ds18b20_temp / 100.0
df$bme280_temp <- df$bme280_temp / 100.0
df$ds3231_temp <- df$ds3231_temp / 100.0
df$bme280_hum <- df$bme280_hum / 10.0
df$bme280_pres <- df$bme280_pres / 10.0
df$adc_bat <- df$adc_bat / 4095 * 3.3 * 2

repeat {

print(unique(as.Date(df$date)))

print(table(as.Date(df$date)))

day <- as.POSIXct(readline("enter day: "), origin="1970-01-01", tz="GMT")
type <- readline("enter plot [1 - temp, 2 - hum, 3 - pres, 4 - bat]: ")

if(!(as.Date(day,tz="GMT")%in%as.Date(df$date,tz="GMT")) || !(type==1 || type==2 || type==3 || type==4)) break;

filter <- (as.Date(df$date,tz="GMT")==as.Date(day,tz="GMT"))
x <- df$date[filter]
l <- length(x)

if(type==1) {
   baly <- "temperatura, C"
   mn <- min(df$ds18b20_temp[filter],df$bme280_temp[filter],df$ds3231_temp[filter])
   mx <- max(df$ds18b20_temp[filter],df$bme280_temp[filter],df$ds3231_temp[filter])
   mily <- c(mn-5, mx+5)
} else if(type==2) {
   baly <- "santykine dregmen, %"
   mn <- min(df$bme280_hum[filter])
   mx <- max(df$bme280_hum[filter])
   mily <- c(mn-10, mx+10)
} else if(type==3) {
   baly <- "slegis, mmHg"
   mn <- min(df$bme280_pres[filter])
   mx <- max(df$bme280_pres[filter])
   mily <- c(mn-10, mx+10)
} else if(type==4) {
   baly <- "baterija, V"
   mn <- min(df$adc_bat[filter])
   mx <- max(df$adc_bat[filter])
   mily <- c(mn-1,mx+1)
}

plot(0, 0, type="n", main=day, xlab="laikas", ylab=baly, xlim=c(day, day+86400), ylim=mily, xaxt="n")
axis(side=1, at=seq(day, day+86400, len=9), labels=format(seq(day, day+86400, len=9),"%H:%M"))
k <- 1
for(i in 2:(l+1)) {
   if(i==l+1 || x[i] != x[i-1]+60) {
      if(type==1) {
         lines((df$date[filter])[k:(i-1)], (df$ds18b20_temp[filter])[k:(i-1)], lwd=2, col="red")
         lines((df$date[filter])[k:(i-1)], (df$bme280_temp[filter])[k:(i-1)], lwd=2, col="blue")
         lines((df$date[filter])[k:(i-1)], (df$ds3231_temp[filter])[k:(i-1)], lwd=2, col="green")
      } else if(type==2) {
         lines((df$date[filter])[k:(i-1)], (df$bme280_hum[filter])[k:(i-1)], lwd=2, col="blue")
      } else if(type==3) {
         lines((df$date[filter])[k:(i-1)], (df$bme280_pres[filter])[k:(i-1)], lwd=2, col="red")
      } else if(type==4) {
         lines((df$date[filter])[k:(i-1)], (df$adc_bat[filter])[k:(i-1)], lwd=2, col="black")
      }
      abline(v=x[k], lty=2)
      abline(v=x[i-1], lty=2)
      text(x[k], par('usr')[3], format(x[k],"%H:%M",tz="GMT"), pos=3)
      text(x[i-1], par('usr')[3], format(x[i-1],"%H:%M",tz="GMT"), pos=3)
      k <- i
   }
}

if(type==1) {
   for(t in 2:4) {
      for(i in 1:2) {
         m <- which(df[,t] == ifelse(i==1,mn,mx))
         if(length(m)>0) {
            points(df$date[m[1]], df[m[1],t], pch=19, col="black")
            text(df$date[m[1]], df[m[1],t], sprintf("%.1f [%s]", df[m[1],t], format(df$date[m[1]], "%H:%M")), pos=ifelse(i==1,1,3))
         }
      }
   }
   legend("bottomright", legend=c("ds18b20", "bme280", "ds3231"), col=c("red", "blue", "green"), lwd=2)
} else if(type==2) {
   for(i in 1:2) {
      m <- which(df$bme280_hum == ifelse(i==1,mn,mx))
      points(df$date[m[1]], df$bme280_hum[m[1]], pch=19, col="black")
      text(df$date[m[1]], df$bme280_hum[m[1]], sprintf("%.1f [%s]", df$bme280_hum[m[1]], format(df$date[m[1]], "%H:%M")), pos=ifelse(i==1,1,3))
   }
   legend("bottomright", legend="bme280", col="blue", lwd=2)
} else if(type==3) {
   for(i in 1:2) {
      m <- which(df$bme280_pres == ifelse(i==1,mn,mx))
      points(df$date[m[1]], df$bme280_pres[m[1]], pch=19, col="black")
      text(df$date[m[1]], df$bme280_pres[m[1]], sprintf("%.1f [%s]", df$bme280_pres[m[1]], format(df$date[m[1]], "%H:%M")), pos=ifelse(i==1,1,3))
   }
   legend("bottomright", legend="bme280", col="red", lwd=2)
} else if(type==4) {
   for(i in 1:2) {
      m <- which(df$adc_bat == ifelse(i==1,mn,mx))
      points(df$date[m[1]], df$adc_bat[m[1]], pch=19, col="black")
      text(df$date[m[1]], df$adc_bat[m[1]], sprintf("%.2f [%s]", df$adc_bat[m[1]], format(df$date[m[1]], "%H:%M")), pos=ifelse(i==1,1,3))
   }
   legend("bottomright", legend="adc", col="black", lwd=2)
}

} #repeat
