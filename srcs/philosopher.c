/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosopher.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: caboudar <caboudar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/26 18:19:40 by caboudar          #+#    #+#             */
/*   Updated: 2023/01/05 06:15:01 by caboudar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosopher.h"

void	launch_philo_routine(t_philo **philo_lst, t_data *data)
{
	t_philo		*philo;
	int			i;

	philo = *philo_lst;
	i = 0;
	while (i < data->nb_of_philos)
	{
		pthread_create(&philo->thread, NULL, (void*)&philo_routine, philo);
		philo = philo->next;
		i++;
		usleep(50);
	}
	// Usleep to synchronise the moment when we check the routine out
	usleep(50);
}

void	routine_exec_if_one_philo_only(t_philo *philo)
{
	if (philo->data->nb_of_philos > 1)
		return ;
	pthread_mutex_lock(&philo->left_fork);
	mutex_print(philo, ACTION_ID,"has taken a fork\n");
	pthread_mutex_unlock(&philo->left_fork);
	usleep(philo->data->time_to_die);
	pthread_mutex_lock(&philo->data->philo_has_died_mutex);
	philo->data->philo_has_died = true;
	pthread_mutex_unlock(&philo->data->philo_has_died_mutex);
	usleep(philo->data->time_to_die);
	mutex_print(philo, DIE_ID, "has died\n");
}

void	philo_routine(t_philo *philo)
{
	routine_exec_if_one_philo_only(philo);
	if (philo->data->nb_of_philos % 2 == 0)
		desync_action_for_even_philo_count(philo);
	else
		desync_action_for_odd_philo_count(philo);
	while (!philo_died(philo)/* ||  !all_philo_full(philo)*/)
	{
		if (!philo_is_eating(philo) || !philo_is_sleeping(philo)
		|| !philo_is_thinking(philo))
			return ;
		usleep(100);
	}
}

void	stop_routine_if_philo_dead_or_full(t_philo **philo_lst, t_data *data)
{
	t_philo		*philo;
	int			i;

	if ((*philo_lst)->data->nb_of_philos == 1)
		return ;
	philo = *philo_lst;
	while (!philo_died(philo) /*|| !all_philo_full(philo)*/)
	{
		philo = *philo_lst;
		i = 0;
		while (i < data->nb_of_philos)
		{
			if (kill_philo_if_possible(philo)
				/*|| try_to_set_all_philo_to_full(philo, data)*/)
					return ;
			i++;
			philo = philo->next;
		}
	}
}

int main(int ac, char **av)
{
	t_philo		*philo;
	t_data		data;

	if (exit_if_args_errors(ac, av) == ERROR)
		return (1);
	if (init_data(&philo, &data, ac, av) == ERROR)
		return (1);
	launch_philo_routine(&philo, &data);
	stop_routine_if_philo_dead_or_full(&philo, &data);
	if (destroy_threads(&philo, &data) == ERROR)
		return (1);
	destroy_mutex(&philo, &data);
	free_linked_list(&philo, &data);
}
