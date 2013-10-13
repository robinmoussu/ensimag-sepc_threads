
data= read.csv("../mesures/mesures.csv",sep=";")

library(ggplot2);


d = data.frame(nvilles=data$nvilles, graines=data$graine, nthreads=data$nthreads, temps=data$temps, coupures=data$coupures, accel=data$temps/data$nthreads);

p = ggplot(d, aes(y=temps,x=nthreads)) + geom_line() + facet_grid(graines ~ nvilles)
p
ggsave("temps.svg",width=2*par("din")[1])

p = ggplot(d, aes(y=accel,x=nthreads)) + geom_line() + facet_grid(graines ~ nvilles)
p
ggsave("accel.svg",width=2*par("din")[1])
