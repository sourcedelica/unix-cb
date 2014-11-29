int m_ruload()
{
	int i, uload;

	uload = 0;

	for (i=0; i<ctl->numslots ;i++)
		if (hotlog[i].status & STF_TAKEN &&
			hotlog[i].status & STF_ACTIVE &&
			hotlog[i].status & STF_DIALUP &&
			!(hotlog[i].status & STF_SYSTEMSLOT) )
			uload++;
	return(uload);
}

int m_uload()
{
	int i, uload;

	uload = 0;

	for (i=0; i<ctl->numslots ;i++)
		if (hotlog[i].status & STF_TAKEN &&
			hotlog[i].status & STF_ACTIVE &&
			!(hotlog[i].status & STF_SYSTEMSLOT) )
			uload++;
	return(uload);
}
