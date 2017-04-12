//Sets a limit on the branching fraction of a decay at a significance passed to the function.
//Starts with a poisson distribution that has mean equal to the observed number of events. 
//It then incriments the mean until the probability of observing less than the number of observed 
//		events passed to the function is less than (1 - significance).
//Uses prob(obs < k) = e^(-mean) SUM[(mean^k)/k!]
//
//Takes input of the number of B->K,mu,e decays observed, the total nuber of events and the significance level
//Prints output of an upper limit on the branching fraction for this decay
//
//NOTE: This only works up to an obs = 31. At obs = 32 it breaks.
void SetLimit(const Int_t obs, const Int_t events, Double_t significance) {
	
	if (obs > 31) {
		std::cout << "The observed number of events must be less than 32.\nExiting.\n";
		return;
	}
	
	Int_t i, j, kfact;
	
	//Calculate the value of i factorial up to i = k 
	Int_t *factorial = (Int_t*)malloc((obs + 1) * sizeof(Int_t));
	factorial[0] = 1;
	for (i = 1; i <= obs; i = i+1) {
		factorial[i] = factorial[i-1] * i;
	}

	Double_t mean = obs;
	Double_t prob = 1.0;
	Double_t prob_tmp;
	while (prob > 1.0-significance) {
		//Calculate the probability of observing less than "obs" number of events
		//Uses prob = e^(-mean) SUM[(mean^i)/i!]
		//Where the sum is from 0 to the observed events
		prob_tmp = 0;
		for (i = 0; i <= obs; i = i + 1) {
			prob_tmp += pow(mean,i) / factorial[i];
		}
		prob_tmp = prob_tmp*exp((-1)*mean);
		//std::cout << "mean = " << mean << ", probability = " << prob_tmp << std::endl;
		//If the probability is greater than the required significance go to the next mean
		if (prob_tmp > 1.0-significance) mean = mean+1;
		prob = prob_tmp;
	}
	std::cout << "The limit on the branching fraction at a significance of: " << significance
			  << " is: " << mean << std::endl;
	return;
}
