/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_exit_status.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sel-hasn <sel-hasn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 10:01:40 by sel-hasn          #+#    #+#             */
/*   Updated: 2024/08/18 18:30:48 by sel-hasn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../mini_shell.h"

int	ft_strncmp(char *s1, const char *s2, size_t n)
{
	size_t			i;
	unsigned char	*str1;
	unsigned char	*str2;

	str1 = (unsigned char *)s1;
	str2 = (unsigned char *)s2;
	i = 0;
	while (i < n && (str1[i] != '\0' && str2[i] != '\0'))
	{
		if (str1[i] != str2[i])
			return (str1[i] - str2[i]);
		i++;
	}
	return (0);
}

int	check_induble(char *s, int i)
{
	int	j;
	int	k;

	j = 0;
	while (s[j] != '\0')
	{
		if (s[j] == '"')
		{
			k = j;
			j++;
			while (s[j] != '\0' && s[j] != '"')
				j++;
			if (s[j] == '"' && (k < i && i < j))
				return (1);
			j++;
		}
		else
			j++;
	}
	return (0);
}

int	ft_exit_update(char **var, int i, char *exit_str, char *secend_part)
{
	char	*first_part;
	char	*update_var;

	first_part = NULL;
	if (i == 0)
		first_part = ft_strdup(exit_str);
	else if (i != 0)
		first_part = ft_strndup(*var, i);
	if (i != 0 && first_part)
		first_part = ft_strjoin(first_part, exit_str);
	if (!first_part)
		return (free(exit_str), -1);
	free(exit_str);
	update_var = ft_strjoin(first_part, secend_part);
	if (!update_var)
	{
		free(first_part);
		return (-1);
	}
	free(*var);
	*var = update_var;
	return (0);
}

int	ft_expand_exit_status(char **var, int exit_status)
{
	int		i;
	char	*exit_str;

	i = 0;
	exit_str = NULL;
	while (var[0][i] != '\0')
	{
		if (var[0][i] == '\'')
			i = ft_skipe_qoute(*var, i);
		else if (var[0][i] == '$' && var[0][i + 1] == '?')
		{
			exit_str = ft_itoa(exit_status);
			if (!exit_str)
				return (-1);
			if (ft_exit_update(var, i, exit_str, &var[0][i + 2]) == -1)
				return (-1);
		}
		else
			i++;
	}
	return (0);
}
